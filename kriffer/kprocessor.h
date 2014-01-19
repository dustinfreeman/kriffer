#include <Winbase.h> //for time functions.

#include <NuiApi.h>
#include <utility.h>
#include <nuistream.h>
#include <nuidepthstream.h>
#include <nuicolourstream.h>
#include <nuiskeletonstream.h>

#include <riffer.h>


#include "lzfx\lzfx.h"

namespace kfr {
	using namespace rfr;

	struct KProcessor {
		CaptureSession cs;
		int k_index;

		INuiSensor * pNuiSensor;

		HRESULT hr;

		NuiDepthStream*			depth_stream;
		NuiColourStream*		colour_stream;
		NuiSkeletonStream*		skeleton_stream;

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

		KProcessor(int _k_index, CaptureSession _cs) {
			cs = _cs;
			k_index = _k_index;

			register_tags();
			cs.index_by("timestamp");

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

				depth_stream->init();
				colour_stream->init();
				skeleton_stream->init();
			}
		}

		void add_current_time(Chunk* chunk) {
			FILETIME time; GetSystemTimeAsFileTime(&time);
			//Contains a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC).
			
			//conversion:
			LARGE_INTEGER foo;
			__int64 bar;
			foo.HighPart = time.dwHighDateTime;
			foo.LowPart = time.dwLowDateTime;
			bar = foo.QuadPart;
			chunk->add_parameter("timestamp", bar);
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

		}
		void ProcessDepth() {
			Chunk depthChunk("depth frame");
			add_current_time(&depthChunk);

			NUI_IMAGE_FRAME imageFrame;
			pNuiSensor->NuiImageStreamGetNextFrame(depth_stream->streamHandle, 0, &imageFrame);
			if (FAILED(hr))
			{
				std::cout << "NuiImageStreamGetNextFrame Failed.\n";
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

				int padding_factor = 4;
				unsigned int olen = lockedRect.size*sizeof(BYTE)*padding_factor;
				void* obuf = malloc(olen);
				int h = lzfx_compress(lockedRect.pBits, lockedRect.size, obuf, &olen);
				if (h < 0) {
					std::cout << "lzfx_compress failed.\n";
					goto ReleaseTexture;
				}
				
				add_resolution(&depthChunk, imageFrame.eResolution);
				//QuadPart is to get int64 from LARGE_INTEGER
				depthChunk.add_parameter("kinect timestamp", imageFrame.liTimeStamp.QuadPart);
				depthChunk.add_parameter("depth image", obuf, olen); 

				cs.add(depthChunk);
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

		void update() {
			if (WAIT_OBJECT_0 == WaitForSingleObject(colour_stream->frameEvent, 0))
			{
				ProcessColor();
			}
			if (WAIT_OBJECT_0 == WaitForSingleObject(depth_stream->frameEvent, 0) )
			{
				ProcessDepth();
			}
			if (WAIT_OBJECT_0 == WaitForSingleObject(skeleton_stream->frameEvent, 0))
			{
				ProcessSkeleton();
			}
		}

		void stop() {
			//stop pushing to capture session and wrap up.
			//capture session stays open.
			colour_stream->close();
			depth_stream->close();
			skeleton_stream->close();

			SafeRelease(pNuiSensor);
		}
	};
};