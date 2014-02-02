#include <Windows.h>
#include <Winbase.h> //for time functions.

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <tchar.h>

#include <sstream>
#include <streambuf>

#include <stdint.h>

#include <NuiApi.h>
#include <utility.h>
#include <nuistream.h>
#include <nuidepthstream.h>
#include <nuicolourstream.h>
#include <nuiskeletonstream.h>
#include <nuiaudio.h>

#include "img_chunk.h"

#include "processor.h"
#include "lzfx\lzfx.h"

#pragma once
#define KINECT_MAX_USERS 6 
//I'm annoyed I couldn't find the above in NuiApi.h

void SignalHandler(int signal)
{
	printf("Signal %d",signal);
	throw "!Access Violation!";
}

namespace kfr {

	using namespace rfr;

	class KProcessor: public Processor {
	public:
		int k_index;

		INuiSensor * pNuiSensor;

		HRESULT hr;

		NuiDepthStream*			depth_stream;
		NuiColourStream*		colour_stream;
		NuiSkeletonStream*		skeleton_stream;
		NuiAudio*				audio_stream;

		ImgChunk* _last_depth;
		ImgChunk* _last_colour;

		float _last_audio_angle;
		std::iostream *output_audio_buffer;

		void register_tags() {
			//ensures tags are registered with riffer
			tags::register_tag("width", "WDTH", INT_TYPE);
			tags::register_tag("height", "HGHT", INT_TYPE);
			tags::register_tag("timestamp", "MTMP", INT_64_TYPE);
			tags::register_tag("kinect timestamp", "TMSP", INT_64_TYPE);
			
			tags::register_tag("depth frame", "DPTH", CHUNK_TYPE);
			tags::register_tag("depth image", "DPDT", CHUNK_TYPE);
			
			tags::register_tag("colour frame", "CLUR", CHUNK_TYPE);
			tags::register_tag("colour image", "CLRI", CHAR_PTR_TYPE);
		}

		KProcessor(int _k_index, std::string _filename = "./capture.dat", bool overwrite = true) 
			: Processor(_filename, overwrite) {
			
			k_index = _k_index;
			//-ve k_index indicates not to open a Kinect. for testing.

			register_tags();

			pNuiSensor = nullptr;

			colour_stream = nullptr;
			depth_stream = nullptr;
			skeleton_stream = nullptr;
			audio_stream = nullptr;

			output_audio_buffer = nullptr;

			_last_depth = nullptr;
			_last_colour = nullptr;
			_last_audio_angle = 0;

			cs->index_by("timestamp");

			if (k_index >= 0) {
				//The following from DepthBasics-D2D CDepthBasics::CreateFirstConnected()
				//open kinect
				hr = NuiCreateSensorByIndex(k_index, &pNuiSensor);
				if (FAILED(hr))
				{
					std::cout << "Could not open Kinect " << k_index << "\n";
				}

				// Get the status of the sensor, and if connected, then we can initialize it
				hr = pNuiSensor->NuiStatus();
				if (S_OK != hr)
				{
					std::cout << "Some error with Kinect " << k_index << "\n";
				}

				//initialize
				hr = pNuiSensor->NuiInitialize(
					NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX
					| NUI_INITIALIZE_FLAG_USES_SKELETON
					| NUI_INITIALIZE_FLAG_USES_COLOR
					| NUI_INITIALIZE_FLAG_USES_AUDIO);

				if (SUCCEEDED(hr))
				{
					depth_stream = new NuiDepthStream(pNuiSensor);
					colour_stream = new NuiColourStream(pNuiSensor);
					skeleton_stream = new NuiSkeletonStream(pNuiSensor);
					audio_stream = new NuiAudio(pNuiSensor);

					depth_stream->init();
					colour_stream->init();
					skeleton_stream->init();
					audio_stream->init();
				}
			}
		}
		
		void add_resolution(Chunk* chunk, int resolution) {
			switch(resolution) {
				case NUI_IMAGE_RESOLUTION_80x60:
					chunk->add_parameter("width",80);
					chunk->add_parameter("height",60);
					break;
				case NUI_IMAGE_RESOLUTION_320x240:
					chunk->add_parameter("width",320);
					chunk->add_parameter("height",240);
					break;
				case NUI_IMAGE_RESOLUTION_640x480:
					chunk->add_parameter("width",640);
					chunk->add_parameter("height",480);
					break;
				case NUI_IMAGE_RESOLUTION_1280x960:
					chunk->add_parameter("width",1280);
					chunk->add_parameter("height",960);
					break;
			}
		}

		//Method from KinectExplorer-D2D
		void ProcessColor() {
			ImgChunk* colourChunk = new ImgChunk("colour frame");
			add_current_time(colourChunk);

			NUI_IMAGE_FRAME imageFrame;

			// Attempt to get the color frame
			hr = pNuiSensor->NuiImageStreamGetNextFrame(colour_stream->streamHandle, 0, &imageFrame);
			if (FAILED(hr))
			{
				std::cout << "Colour's NuiImageStreamGetNextFrame Failed.\n";
				return;
			}

			INuiFrameTexture* pTexture = imageFrame.pFrameTexture;

			// Lock the frame data so the Kinect knows not to modify it while we are reading it
			NUI_LOCKED_RECT lockedRect;
			pTexture->LockRect(0, &lockedRect, NULL, 0);

			// Make sure we've received valid data
			if (lockedRect.Pitch != 0)
			{
				add_resolution(colourChunk, imageFrame.eResolution);

				//QuadPart is to get int64 from LARGE_INTEGER
				colourChunk->add_parameter("kinect timestamp", imageFrame.liTimeStamp.QuadPart);

				colourChunk->assign_image(lockedRect.pBits, lockedRect.size*sizeof(BYTE));
				
				int olen;
				char* comp_img = Processor::compress_image(colourChunk, &olen);

				if(colourChunk->valid_compression) {
					cs->add(*colourChunk);

					//set last colour
					if (_last_colour != nullptr)
						delete _last_colour;
					_last_colour = colourChunk;
				} else {
					std::cout << "Problem with jpge compression. \n";
				}
				
			}

			// Unlock frame data
			pTexture->UnlockRect(0);
			pNuiSensor->NuiImageStreamReleaseFrame(colour_stream->streamHandle, &imageFrame);
		}
		void ProcessDepth() {
			ImgChunk* depthChunk = new ImgChunk("depth frame");
			add_current_time(depthChunk);

			NUI_IMAGE_FRAME imageFrame;
			pNuiSensor->NuiImageStreamGetNextFrame(depth_stream->streamHandle, 0, &imageFrame);
			if (FAILED(hr))
			{
				std::cout << "Depth's NuiImageStreamGetNextFrame Failed.\n";
				return;
			}

			// Get the depth image pixel texture
			BOOL nearMode;
			INuiFrameTexture* pTexture;
			hr = pNuiSensor->NuiImageFrameGetDepthImagePixelFrameTexture(depth_stream->streamHandle, &imageFrame, &nearMode, &pTexture);
			if (FAILED(hr))
			{
				goto ReleaseFrame;
			}

			// Lock the frame data so the Kinect knows not to modify it while we're reading it
			NUI_LOCKED_RECT lockedRect;
			pTexture->LockRect(0, &lockedRect, NULL, 0);

			// Make sure we've received valid data
			if (lockedRect.Pitch != 0)
			{
				// Convert depth data to color image and copy to image buffer
				//m_imageBuffer.CopyDepth(lockedRect.pBits, lockedRect.size, nearMode, m_depthTreatment);

				depthChunk->assign_image(lockedRect.pBits, lockedRect.size*sizeof(BYTE));

				unsigned int olen = depthChunk->image_size*PADDING_FACTOR;
				void* obuf = malloc(olen);
				int h = lzfx_compress(depthChunk->image, depthChunk->image_size, obuf, &olen);
				if (h < 0) {
					std::cout << "lzfx_compress failed.\n";
					goto ReleaseTexture;
				} else {
				
					add_resolution(depthChunk, imageFrame.eResolution);
					//QuadPart is to get int64 from LARGE_INTEGER
					depthChunk->add_parameter("kinect timestamp", imageFrame.liTimeStamp.QuadPart);
					depthChunk->add_parameter("depth image", obuf, olen); 

					//set last depth
					if (_last_depth != nullptr)
						delete _last_depth;
					_last_depth = depthChunk;

					cs->add(*depthChunk);
				}
				delete obuf;
			}

		ReleaseTexture:

			// Done with the texture. Unlock and release it
			pTexture->UnlockRect(0);
			pTexture->Release();

		ReleaseFrame:
			// Release the frame
			pNuiSensor->NuiImageStreamReleaseFrame(depth_stream->streamHandle, &imageFrame);

		}
		void ProcessSkeleton() {

		}
		
		typedef void (*SignalHandlerPointer)(int);

		void ProcessAudio() {
			
			return; //HACK NO AUDIO FOR NOW.
			
			ULONG cbProduced = 0;
			BYTE *pProduced = NULL;
			DWORD dwStatus = 0;
			DMO_OUTPUT_DATA_BUFFER outputBuffer = {0};
			outputBuffer.pBuffer = &audio_stream->buffer;

			if (output_audio_buffer)
				std::cout << "buffer pos: " << output_audio_buffer->tellp() << "\n";

			HRESULT hr = S_OK;
			//std::cout << "loop ";
			
			do
			{
				audio_stream->buffer.Init(0);
				outputBuffer.dwStatus = 0;

				//http://stackoverflow.com/a/918891/2518451
				SignalHandlerPointer previousHandler;
				previousHandler = signal(SIGSEGV , SignalHandler);
				try {
					//receive confusing access violation errors in line below
					//catching by: http://stackoverflow.com/a/8234956/2518451
					//std::cout << "here ";
					hr = audio_stream->m_pDMO->ProcessOutput(0, 1, &outputBuffer, &dwStatus);
				} catch (char *e) {
					std::cout << "Caught exception in ProcessAudio() " << e << "\n";
					break;
				}

				//std::cout << "there \n";

				if (FAILED(hr))
				{
					std::cout << "Failed to process audio output. \n";
					break;
				}

				if (hr == S_FALSE)
				{
					cbProduced = 0;
				}
				else
				{
					audio_stream->buffer.GetBufferAndLength(&pProduced, &cbProduced);
				}

				if (cbProduced > 0)
				{
					double beamAngle, sourceAngle, sourceConfidence;

					// Obtain beam angle from INuiAudioBeam afforded by microphone array
					audio_stream->source->GetBeam(&beamAngle);
					audio_stream->source->GetPosition(&sourceAngle, &sourceConfidence);

					_last_audio_angle = sourceAngle;
					//std::cout << _last_audio_angle << "\n";

					if (output_audio_buffer) {
						output_audio_buffer->write((const char*)pProduced, cbProduced);
						std::cout << "writing to buffer " << cbProduced << "/" << output_audio_buffer->tellp() << "\n";
					}

				}
				//std::cout << ".";

			} while (outputBuffer.dwStatus & DMO_OUTPUT_DATA_BUFFERF_INCOMPLETE);
			//std::cout << " -- \n";
		}

		std::string update() {
			ProcessAudio(); //I feel like this should be called more frequently?

			std::ostringstream new_frames;
			if (WAIT_OBJECT_0 == WaitForSingleObject(colour_stream->frameEvent, 0))
			{
				ProcessColor();
				new_frames << "c";
			}

			//HACK ONLY COLOUR FRAMES FOR NOW
			// if we use other frame types, 
			//	we will have to adjust indexing so it can filter a specific frame type.
			/*if (WAIT_OBJECT_0 == WaitForSingleObject(depth_stream->frameEvent, 0) )
			{
				ProcessDepth();
				new_frames << "d";
			}
			if (WAIT_OBJECT_0 == WaitForSingleObject(skeleton_stream->frameEvent, 0))
			{
				ProcessSkeleton();
				new_frames << "s";
			}*/
			return new_frames.str();
		}

		ImgChunk* last_depth() {
			return _last_depth;
		}

		ImgChunk* last_colour() {
			return _last_colour;
		}

		

		float last_audio_angle() {
			return _last_audio_angle;
		}

		void stop() {
			//stop pushing to capture session and wrap up.
			//capture session stays open.
			if (colour_stream)
				colour_stream->close();
			if (depth_stream)
				depth_stream->close();
			if (skeleton_stream)
				skeleton_stream->close();
			if (audio_stream)
				audio_stream->close();

			SafeRelease(pNuiSensor);

			Processor::stop();
		}
	};
};