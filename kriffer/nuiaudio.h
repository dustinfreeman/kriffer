#pragma once

#include "audio_utils.h"
#include "WASAPICapture.h"

//mostly from AudioBasics-D2D
struct NuiAudio {
	INuiSensor* pNuiSensor;

	// Audio source used to query Kinect audio beam and sound source angles.
    INuiAudioBeam*          source;

	IMMDevice *device;

	CWASAPICapture *capturer;

	HANDLE waveFile;

	bool capturing;

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
		wchar_t ch;

		// Write a placeholder wave file header. Actual size of data section will be fixed up later.
		hr = WriteWaveHeader(waveFile, capturer->GetOutputFormat(), 0);
		if (SUCCEEDED(hr))
		{
			if (capturer->Start(waveFile))
			{
				printf_s("Capturing audio data to file %S\nPress 's' to stop capturing.\n", waveFileName);
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

	void init() {
		HRESULT hr;

		// Get the audio source (for angle and beam stuff)
		hr = pNuiSensor->NuiGetAudioSource(&source);
		if (!SUCCEEDED(hr))
		{
			std::cout << "NuiGetAudioSource Failed \n";
		}

		//  Find the audio device corresponding to the kinect sensor.
        hr = GetMatchingAudioDevice(pNuiSensor, &device);
        if (SUCCEEDED(hr))
        {
			//audio recording is initialized individually.
        }
        else
        {
            printf_s("No matching audio device found!\n");
        }
		
	}

	void start_index(int audio_index) {
		HRESULT hr;

		wchar_t waveFileName[MAX_PATH];
		StringCchPrintfW(waveFileName, _countof(waveFileName), L"C:\\Users\\dustin\\Documents\\Code\\improv_remix\\improv_remix\\nuiaudio-%i.wav", audio_index);

		// Create the wave file that will contain audio data
            waveFile = CreateFile(waveFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 
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