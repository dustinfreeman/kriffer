#include <NuiApi.h>

#include <riffer.h>

#include <utility.h>

namespace kfr {
	using namespace rfr;

	struct KProcessor {
		CaptureSession cs;

		INuiSensor * pNuiSensor;

		HRESULT hr;
		HANDLE                  m_pDepthStreamHandle;
		HANDLE                  m_hNextDepthFrameEvent;

		HANDLE                  m_pColourStreamHandle;
		HANDLE                  m_hNextColourFrameEvent;

		HANDLE                  m_pSkeletonStreamHandle;
		HANDLE                  m_hNextSkeletonFrameEvent;

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
				// Create an event that will be signaled when depth data is available
				m_hNextDepthFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
				// Open a depth image stream to receive depth frames
				hr = pNuiSensor->NuiImageStreamOpen(
					NUI_IMAGE_TYPE_DEPTH,
					NUI_IMAGE_RESOLUTION_640x480,
					0,
					2,
					m_hNextDepthFrameEvent,
					&m_pDepthStreamHandle);

				// Create an event that will be signaled when depth data is available
				m_hNextColourFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
				// Open a colour image stream to receive colour frames
				hr = pNuiSensor->NuiImageStreamOpen(
					NUI_IMAGE_TYPE_COLOR,
					NUI_IMAGE_RESOLUTION_640x480,
					0,
					2,
					m_hNextColourFrameEvent,
					&m_pColourStreamHandle);

				// Create an event that will be signaled when depth data is available
				m_hNextSkeletonFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
				DWORD flags = 0;
				//http://msdn.microsoft.com/en-us/library/nuisensor.inuisensor.nuiskeletontrackingenable.aspx
				/*(m_seated ? NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT : 0) | (m_near ? NUI_SKELETON_TRACKING_FLAG_ENABLE_IN_NEAR_RANGE : 0)
                | (ChooserModeDefault != m_chooserMode ? NUI_SKELETON_TRACKING_FLAG_TITLE_SETS_TRACKED_SKELETONS : 0);*/
				pNuiSensor->NuiSkeletonTrackingEnable(m_hNextSkeletonFrameEvent, flags);
			}
		}

		void ProcessDepth() {
			//retrieve Depth Data from Stream
		}
		void ProcessColor() {

		}
		void ProcessSkeleton() {

		}

		void update() {
			if ( WAIT_OBJECT_0 == WaitForSingleObject(m_hNextDepthFrameEvent, 0) )
			{
				ProcessDepth();
			}
			if (WAIT_OBJECT_0 == WaitForSingleObject(m_hNextColourFrameEvent, 0))
			{
				ProcessColor();
			}
			if (WAIT_OBJECT_0 == WaitForSingleObject(m_hNextSkeletonFrameEvent, 0))
			{
				ProcessSkeleton();
			}
		}

		void stop() {
			//stop pushing to capture session and wrap up.
			//capture session stays open.
			CloseHandle(m_hNextDepthFrameEvent);
			CloseHandle(m_hNextColourFrameEvent);
			CloseHandle(m_hNextSkeletonFrameEvent);
			SafeRelease(pNuiSensor);
		}
	};
};