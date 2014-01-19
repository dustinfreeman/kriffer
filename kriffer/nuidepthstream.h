#include <nuistream.h>

struct NuiDepthStream : NuiStream {
	bool            m_nearMode;

	NuiDepthStream(INuiSensor * _pNuiSensor):
		NuiStream(_pNuiSensor)	,
		m_nearMode(false)
	{ 
		pNuiSensor = _pNuiSensor;
	}

	void init() {
		// Create an event that will be signaled when depth data is available
		//frameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		frameEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

		// Open a depth image stream to receive depth frames
		hr = pNuiSensor->NuiImageStreamOpen(
			NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,
			NUI_IMAGE_RESOLUTION_640x480,
			0,
			2,
			frameEvent,
			&streamHandle);

		if (SUCCEEDED(hr))
		{
			pNuiSensor->NuiImageStreamSetImageFrameFlags(streamHandle, m_nearMode ? NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE : 0);   // Set image flags
		}
	}
};