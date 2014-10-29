#pragma once

#include "audio_buffer.h"
#include "audio_utils.h"
//#include "WASAPICapture.h"



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
    CStaticMediaBuffer buffer;

	IMMDevice *device;

	CWASAPICapture *capturer;
	bool capturing;

	std::string folder;

	HANDLE waveFile;
	wchar_t waveFileName[MAX_PATH];

	std::string get_wav_filename() {
		std::wstring ws(waveFileName);
		return std::string(ws.begin(), ws.end());
	}

	NuiAudio(INuiSensor * _pNuiSensor = nullptr) {
		pNuiSensor = _pNuiSensor;
		device = NULL;
		capturer = NULL;
		waveFile = INVALID_HANDLE_VALUE;
		capturing = false;
	}

	HRESULT CaptureAudio(CWASAPICapture *capturer, HANDLE waveFile, const wchar_t *waveFileName)
	{
		HRESULT hr = S_OK;

		// Write a placeholder wave file header. Actual size of data section will be fixed up later.
		hr = WriteWaveHeader(waveFile, capturer->GetOutputFormat(), 0);
		if (SUCCEEDED(hr))
		{
			if (capturer->Start(waveFile))
			{
				//printf_s("Capturing audio data to file %S\nPress 's' to stop capturing.\n", waveFileName);
				capturing = true;
				//do
				//{
				//	//infinite capture loop.
				//} while (capturing);

				//printf_s("\n");
			}
			else
			{
				hr = E_FAIL;
			}
		}

		return hr;
	}

	void init_source_beam() {
		HRESULT hr;

		// Get the audio source (for angle and beam stuff)
		hr = pNuiSensor->NuiGetAudioSource(&source);
		if (!SUCCEEDED(hr))
		{
			std::cout << "NuiGetAudioSource Failed \n";
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
		// SINGLE_CHANNEL_AEC = 0
		// OPTIBEAM_ARRAY_ONLY = 2
		// OPTIBEAM_ARRAY_AND_AEC = 4
		// SINGLE_CHANNEL_NSAGC = 5
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
		if (FAILED(hr))
		{
			std::cout << "Audio FAILED(hr) \n";
		}
		MoFreeMediaType(&mt);
	}

	void init() {
		HRESULT hr;

		init_source_beam();

		//  Find the audio device corresponding to the kinect sensor.
		CoInitialize(nullptr); // NULL if using older VC++
		//above line added to fix bad hr result from below call. http://stackoverflow.com/a/17076454
        hr = GetMatchingAudioDevice(pNuiSensor, &device);
        if (SUCCEEDED(hr))
        {
			//formerly, audio recording was initialized here - now in start_index()
        }
        else
        {
            printf_s("No matching audio device found!\n");
        }
	}

	void start_index(int audio_index) {
		HRESULT hr;

		std::wstring stemp = std::wstring(folder.begin(), folder.end());
		StringCchPrintfW(waveFileName, _countof(waveFileName), L"%snuiaudio-%i.wav", stemp.c_str(), audio_index);

		// Create the wave file that will contain audio data
		waveFile = CreateFileW(waveFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 
            NULL);

        if (INVALID_HANDLE_VALUE != waveFile)
        {
            //  Instantiate a capturer
            capturer = new (std::nothrow) CWASAPICapture(device);
            if ((NULL != capturer) && capturer->Initialize(TargetLatency))
            {
                hr = CaptureAudio(capturer, waveFile, waveFileName);
                if (FAILED(hr))
                {
                    printf_s("Unable to capture audio data.\n");
                }
            }
            else
            {
                printf_s("Unable to initialize capturer.\n");
                hr = E_FAIL;
            }
        }
        else
        {
            printf_s("Unable to create output WAV file %S.\nAnother application might be using this file.\n", waveFileName);
            hr = E_FAIL;
        }
	}

	void stop_audio() {
		HRESULT hr;

		capturer->Stop();
		// Fix up the wave file header to reflect the right amount of captured data.
		SetFilePointer(waveFile, 0, NULL, FILE_BEGIN);
		hr = WriteWaveHeader(waveFile, capturer->GetOutputFormat(), capturer->BytesCaptured());

		capturing = false;
	}

	void close() {

	}
};