#pragma once

struct NuiStream {
	HANDLE frameEvent;
	HANDLE streamHandle;
	INuiSensor* pNuiSensor;

	HRESULT hr;

	NuiStream(INuiSensor * _pNuiSensor = nullptr) 
		: pNuiSensor(_pNuiSensor) { }

	void init();

	void close() {
		CloseHandle(frameEvent);
	}
};