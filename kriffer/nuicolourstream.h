#include <nuistream.h>

struct NuiColourStream : NuiStream {
	NuiColourStream(INuiSensor * _pNuiSensor) { 
		pNuiSensor = _pNuiSensor;
	}

	void init() {
		// Create an event that will be signaled when depth data is available
		frameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		// Open a colour image stream to receive colour frames
		hr = pNuiSensor->NuiImageStreamOpen(
			NUI_IMAGE_TYPE_COLOR,
			NUI_IMAGE_RESOLUTION_640x480,
			0,
			2,
			frameEvent,
			&streamHandle);
	}
};