#include <NuiApi.h>

#include <riffer.h>

#include <utility.h>
#include <nuistream.h>
#include <nuidepthstream.h>
#include <nuicolourstream.h>
#include <nuiskeletonstream.h>

namespace kfr {
	using namespace rfr;

	struct KProcessor {
		CaptureSession cs;

		INuiSensor * pNuiSensor;

		HRESULT hr;

		NuiDepthStream*			depth_stream;
		NuiColourStream*		colour_stream;
		NuiSkeletonStream*		skeleton_stream;

		KProcessor(int k_index, CaptureSession _cs) {
			cs = _cs;

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

		//TODO create riffer chunks below.
		//See KinectExplorer-D2D
		void ProcessDepth() {

		}
		void ProcessColor() {

		}
		void ProcessSkeleton() {

		}

		void update() {
			if (WAIT_OBJECT_0 == WaitForSingleObject(depth_stream->frameEvent, 0) )
			{
				ProcessDepth();
			}
			if (WAIT_OBJECT_0 == WaitForSingleObject(colour_stream->frameEvent, 0))
			{
				ProcessColor();
			}
			if (WAIT_OBJECT_0 == WaitForSingleObject(skeleton_stream->frameEvent, 0))
			{
				ProcessSkeleton();
			}
		}

		void stop() {
			//stop pushing to capture session and wrap up.
			//capture session stays open.
			depth_stream->close();
			colour_stream->close();
			skeleton_stream->close();

			SafeRelease(pNuiSensor);
		}
	};
};