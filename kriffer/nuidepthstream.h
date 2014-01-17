#include <nuistream.h>

struct NuiDepthStream : NuiStream {
	NuiDepthStream(INuiSensor * _pNuiSensor) { 
		pNuiSensor = _pNuiSensor;
	}

	void init() {
		// Create an event that will be signaled when depth data is available
		frameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		// Open a depth image stream to receive depth frames
		hr = pNuiSensor->NuiImageStreamOpen(
			NUI_IMAGE_TYPE_DEPTH,
			NUI_IMAGE_RESOLUTION_640x480,
			0,
			2,
			frameEvent,
			&streamHandle);
	}
};