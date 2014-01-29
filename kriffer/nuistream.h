#pragma once
#include <NuiApi.h>

struct NuiStream {
	HANDLE frameEvent;
	HANDLE streamHandle;
	INuiSensor* pNuiSensor;

	HRESULT hr;

	NuiStream(INuiSensor * _pNuiSensor = nullptr) 
		: pNuiSensor(_pNuiSensor) { 
		frameEvent = nullptr;	
	}

	void init();

	void close() {
		if (frameEvent)
			CloseHandle(frameEvent);
	}
};