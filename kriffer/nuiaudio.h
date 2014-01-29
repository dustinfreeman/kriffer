#pragma once

#include "audio_utils.h"

//mostly from AudioBasics-D2D
struct NuiAudio {
	INuiSensor* pNuiSensor;

	// Audio source used to query Kinect audio beam and sound source angles.
    INuiAudioBeam*          source;

	// Media object from which Kinect audio stream is captured.
    IMediaObject*           m_pDMO;

    // Property store used to configure Kinect audio properties.
    IPropertyStore*         m_pPropertyStore;

	// Buffer to hold captured audio data.
    CStaticMediaBuffer      buffer;

	NuiAudio(INuiSensor * _pNuiSensor = nullptr) {
		pNuiSensor = _pNuiSensor;
	}

	void init() {
		// Get the audio source
		HRESULT hr = pNuiSensor->NuiGetAudioSource(&source);
		if (FAILED(hr))
		{
			std::cout << "Audio FAILED(hr) \n";
		}

		
		hr = source->QueryInterface(IID_IMediaObject, (void**)&m_pDMO);
		if (FAILED(hr))
		{
			std::cout << "Audio FAILED(hr) \n";
		}

		hr = source->QueryInterface(IID_IPropertyStore, (void**)&m_pPropertyStore);
		if (FAILED(hr))
		{
			std::cout << "Audio FAILED(hr) \n";
		}

		// Set AEC-MicArray DMO system mode. This must be set for the DMO to work properly.
		// Possible values are:
		//   SINGLE_CHANNEL_AEC = 0
		//   OPTIBEAM_ARRAY_ONLY = 2
		//   OPTIBEAM_ARRAY_AND_AEC = 4
		//   SINGLE_CHANNEL_NSAGC = 5
		PROPVARIANT pvSysMode;
		PropVariantInit(&pvSysMode);
		pvSysMode.vt = VT_I4;
		pvSysMode.lVal = (LONG)(2); // Use OPTIBEAM_ARRAY_ONLY setting. Set OPTIBEAM_ARRAY_AND_AEC instead if you expect to have sound playing from speakers.
		m_pPropertyStore->SetValue(MFPKEY_WMAAECMA_SYSTEM_MODE, pvSysMode);
		PropVariantClear(&pvSysMode);

		// Set DMO output format
		
		WAVEFORMATEX wfxOut = {AudioFormat, AudioChannels, AudioSamplesPerSecond, AudioAverageBytesPerSecond, AudioBlockAlign, AudioBitsPerSample, 0};
		DMO_MEDIA_TYPE mt = {0};
		hr = MoInitMediaType(&mt, sizeof(WAVEFORMATEX));
		if (FAILED(hr))
		{
			std::cout << "Audio FAILED(hr) \n";
		}

		mt.majortype = MEDIATYPE_Audio;
		mt.subtype = MEDIASUBTYPE_PCM;
		mt.lSampleSize = 0;
		mt.bFixedSizeSamples = TRUE;
		mt.bTemporalCompression = FALSE;
		mt.formattype = FORMAT_WaveFormatEx;	
		memcpy_s(mt.pbFormat, sizeof(WAVEFORMATEX), &wfxOut, sizeof(WAVEFORMATEX));

		hr = m_pDMO->SetOutputType(0, &mt, 0); 
		MoFreeMediaType(&mt);
		
	}


	void close() {

	}
};