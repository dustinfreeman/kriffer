#include <NuiApi.h>
#include <kr_util.h>
#include <nuistream.h>
#include <nuidepthstream.h>
#include <nuicolourstream.h>
#include <nuiskeletonstream.h>
#include <nuiaudio.h>

#include "kprocessor.h"

#define KINECT_MAX_USERS 6 
//I'm annoyed I couldn't find the above in NuiApi.h

namespace kfr {

	int get_num_kinect_1s() {
		int iSensorCount = 0;
		HRESULT hr = NuiGetSensorCount(&iSensorCount);
		return iSensorCount;
	}

	class K1Processor: public KProcessor {
	friend class KrifferTest;
	
	public:

		//constructor ==========
		K1Processor(int _k_index, 
			std::string _folder = RFR_DEFAULT_FOLDER, 
			std::string _filename = RFR_DEFAULT_FILENAME, 
			int _capture_select= CAPTURE_ALL,
			bool overwrite = true);

		std::string get_wav_filename();
		void start_audio_index(int audio_index) {
			audio_stream->start_index(audio_index);
		}
		void stop_audio() {
			audio_stream->stop_audio();
		}

		void stop();

	protected:
		INuiSensor * pNuiSensor;

		HRESULT hr;

		NuiDepthStream*			depth_stream;
		NuiColourStream*		colour_stream;
		NuiSkeletonStream*		skeleton_stream;
		NuiAudio*				audio_stream;

		void add_resolution(Chunk* chunk, int resolution);

		//Methods from KinectExplorer-D2D
		bool ProcessColor();

		void add_depth_chunk(ImgChunk* depthChunk);
		bool ProcessDepth();

		bool ProcessAudio();
	};

	K1Processor::K1Processor(int _k_index, 
		std::string _folder, 
		std::string _filename, 
		int _capture_select,
		bool overwrite) 
		: KProcessor(_k_index, _folder, _filename, _capture_select, overwrite) {

		pNuiSensor = nullptr;

		colour_stream = nullptr;
		depth_stream = nullptr;
		skeleton_stream = nullptr;
		audio_stream = nullptr;

		kinect_opened = false;
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
				if (capture_select & CAPTURE_DEPTH) {
					depth_stream = new NuiDepthStream(pNuiSensor);
					depth_stream->init();
				}
				if (capture_select & CAPTURE_COLOUR) {
					colour_stream = new NuiColourStream(pNuiSensor);
					colour_stream->init();
				}
	
				if (capture_select & CAPTURE_SKELETON) {
					skeleton_stream = new NuiSkeletonStream(pNuiSensor);
					skeleton_stream->init();
				}	
					
				if (capture_select & CAPTURE_AUDIO) {
					audio_stream = new NuiAudio(pNuiSensor);
					audio_stream->folder = cs->get_folder();
					audio_stream->init();
				}

				kinect_opened = true;
			}
		}
	}
	
	void K1Processor::add_resolution(Chunk* chunk, int resolution) {
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

	bool K1Processor::ProcessColor() {
		bool got_frame = false;

		if (! WAIT_OBJECT_0 == WaitForSingleObject(colour_stream->frameEvent, 0))
			return got_frame;

		ImgChunk* colourChunk = new ImgChunk("colour frame");
		add_current_time(colourChunk);

		NUI_IMAGE_FRAME imageFrame;
		// Attempt to get the color frame
		hr = pNuiSensor->NuiImageStreamGetNextFrame(colour_stream->streamHandle, 0, &imageFrame);
		if (FAILED(hr))
		{
			std::cout << "Colour's NuiImageStreamGetNextFrame Failed.\n";
			return got_frame;
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
			char* comp_img = Processor::compress_image(colourChunk, "colour image", &olen, "JPEG");

			if(colourChunk->valid_compression) {
				cs->add(*colourChunk);

				//set last colour
				if (_last_colour != nullptr)
					delete _last_colour;
				_last_colour = colourChunk;
			} else {
				std::cout << "Problem with jpge compression. \n";
			}
			got_frame = true;
		}

		// Unlock frame data
		pTexture->UnlockRect(0);
		pNuiSensor->NuiImageStreamReleaseFrame(colour_stream->streamHandle, &imageFrame);

		return got_frame;
	}
		
	void K1Processor::add_depth_chunk(ImgChunk* depthChunk) {

		int olen;
		char* comp_img = Processor::compress_image(depthChunk, "depth image", &olen,  "LZF");
		//std::cout << "olen " << olen << "\n";

		if(depthChunk->valid_compression) {
			cs->add(*depthChunk);

			if (_last_depth != nullptr)
				delete _last_depth;
			_last_depth = depthChunk;

		} else {
			std::cout << "Problem with lzf compression. \n";
		}
	}

	bool K1Processor::ProcessDepth() {
		bool got_frame = false;

		if (! WAIT_OBJECT_0 == WaitForSingleObject(depth_stream->frameEvent, 0))
			return got_frame; 

		ImgChunk* depthChunk = new ImgChunk("depth frame");
		add_current_time(depthChunk);

		NUI_IMAGE_FRAME imageFrame;
		pNuiSensor->NuiImageStreamGetNextFrame(depth_stream->streamHandle, 0, &imageFrame);
		if (FAILED(hr))
		{
			std::cout << "Depth's NuiImageStreamGetNextFrame Failed.\n";
			return got_frame;
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
			add_resolution(depthChunk, imageFrame.eResolution);

			depthChunk->add_parameter("kinect timestamp", imageFrame.liTimeStamp.QuadPart);
			depthChunk->assign_image(lockedRect.pBits, lockedRect.size*sizeof(BYTE));

			add_depth_chunk(depthChunk);
			got_frame = true;	
		}

	ReleaseTexture:

		// Done with the texture. Unlock and release it
		pTexture->UnlockRect(0);
		pTexture->Release();

	ReleaseFrame:
		// Release the frame
		pNuiSensor->NuiImageStreamReleaseFrame(depth_stream->streamHandle, &imageFrame);

		return got_frame;
	}

	bool K1Processor::ProcessAudio() {
		ULONG cbProduced = 0;
		BYTE *pProduced = NULL;
		DWORD dwStatus = 0;
		DMO_OUTPUT_DATA_BUFFER outputBuffer = {0};
		outputBuffer.pBuffer = &audio_stream->buffer;

		HRESULT hr = S_OK;
		do
		{
			audio_stream->buffer.Init(0);
			outputBuffer.dwStatus = 0;

			hr = audio_stream->m_pDMO->ProcessOutput(0, 1, &outputBuffer, &dwStatus);

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

				_last_audio_angle = (float)sourceAngle;
				//std::cout << _last_audio_angle << "\n";

				//output_audio_buffer->write((const char*)pProduced, cbProduced);
			}

		} while (outputBuffer.dwStatus & DMO_OUTPUT_DATA_BUFFERF_INCOMPLETE);

		return true;
	}

	std::string K1Processor::get_wav_filename() {
		return audio_stream->get_wav_filename();
	}

	void K1Processor::stop() {
		if (colour_stream)
			colour_stream->close();
		if (depth_stream)
			depth_stream->close();
		if (skeleton_stream)
			skeleton_stream->close();
		if (audio_stream)
			audio_stream->close();

		SafeRelease(pNuiSensor);

		KProcessor::stop();
	}
};