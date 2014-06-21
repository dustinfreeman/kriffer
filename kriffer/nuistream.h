#pragma once
#include <NuiApi.h>

struct NuiStream {
	HANDLE frameEvent;
	HANDLE streamHandle;
	INuiSensor* pNuiSensor;

	HRESULT hr;

	NuiStream(INuiSensor * _pNuiSensor = NULL) 
		: pNuiSensor(_pNuiSensor),
		streamHandle(INVALID_HANDLE_VALUE),
		frameEvent(INVALID_HANDLE_VALUE) 
	{ }

	void init();

	void close() {
		if (frameEvent)
			CloseHandle(frameEvent);
	}
};