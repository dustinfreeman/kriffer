#pragma once

#include "kr_util.h"

#include <nuiapi.h>
#include <shlobj.h>
#include <wchar.h>
#include <devicetopology.h>

#include "WASAPICapture.h"


//  Header for a WAV file - we define a structure describing the first few fields in the header for convenience.
struct WAVEHEADER
{
    DWORD   dwRiff;                     // "RIFF"
    DWORD   dwSize;                     // Size
    DWORD   dwWave;                     // "WAVE"
    DWORD   dwFmt;                      // "fmt "
    DWORD   dwFmtSize;                  // Wave Format Size
};

//  Static RIFF header, we'll append the format to it.
const BYTE WaveHeaderTemplate[] = 
{
    'R',   'I',   'F',   'F',  0x00,  0x00,  0x00,  0x00, 'W',   'A',   'V',   'E',   'f',   'm',   't',   ' ', 0x00, 0x00, 0x00, 0x00
};

//  Static wave DATA tag.
const BYTE WaveData[] = { 'd', 'a', 't', 'a'};

// Number of milliseconds of acceptable lag between live sound being produced and recording operation.
const int TargetLatency = 20;

/// <summary>
/// Get global ID for specified device.
/// </summary>
/// <param name="pDevice">
/// [in] Audio device for which we're getting global ID.
/// </param>
/// <param name="ppszGlobalId">
/// [out] Global ID corresponding to audio device.
/// </param>
/// <returns>
/// S_OK on success, otherwise failure code.
/// </returns>
HRESULT GetGlobalId(IMMDevice *pDevice, wchar_t **ppszGlobalId)
{
    IDeviceTopology *pTopology = NULL;
    HRESULT hr = S_OK;

    hr = pDevice->Activate(__uuidof(IDeviceTopology), CLSCTX_INPROC_SERVER, NULL, reinterpret_cast<void**>(&pTopology));
    if (SUCCEEDED(hr))
    {
        IConnector *pPlug = NULL;

        hr = pTopology->GetConnector(0, &pPlug);
        if (SUCCEEDED(hr))
        {
            IConnector *pJack = NULL;

            hr = pPlug->GetConnectedTo(&pJack);
            if (SUCCEEDED(hr))
            {
                IPart *pJackAsPart = NULL;
                pJack->QueryInterface(IID_PPV_ARGS(&pJackAsPart));

                hr = pJackAsPart->GetGlobalId(ppszGlobalId);
                SafeRelease(pJackAsPart);
            }

            SafeRelease(pPlug);
        }

        SafeRelease(pTopology);
    }

    return hr;
}

/// <summary>
/// Determine if a global audio device ID corresponds to a Kinect sensor.
/// </summary>
/// <param name="pNuiSensor">
/// [in] A Kinect sensor.
/// </param>
/// <param name="pszGlobalId">
/// [in] Global audio device ID to compare to the Kinect sensor's ID.
/// </param>
/// <returns>
/// true if the global device ID corresponds to the sensor specified, false otherwise.
/// </returns>
bool IsMatchingAudioDevice(INuiSensor *pNuiSensor, wchar_t *pszGlobalId)
{
    // Get USB device name from the sensor
    BSTR arrayName = pNuiSensor->NuiAudioArrayId(); // e.g. "USB\\VID_045E&PID_02BB&MI_02\\7&9FF7F87&0&0002"

    wistring strDeviceName(pszGlobalId); // e.g. "{2}.\\\\?\\usb#vid_045e&pid_02bb&mi_02#7&9ff7f87&0&0002#{6994ad04-93ef-11d0-a3cc-00a0c9223196}\\global/00010001"
    wistring strArrayName(arrayName);

    // Make strings have the same internal delimiters
    wistring::size_type findIndex = strArrayName.find(L'\\');
    while (strArrayName.npos != findIndex)
    {
        strArrayName[findIndex] = L'#';
        findIndex = strArrayName.find(L'\\', findIndex + 1);
    }

    // Try to match USB part names for sensor vs audio device global ID
    bool match = strDeviceName.find(strArrayName) != strDeviceName.npos;

    SysFreeString(arrayName);
    return match;
}


/// <summary>
/// Write the WAV file header contents. 
/// </summary>
/// <param name="waveFile">
/// [in] Handle to file where header will be written.
/// </param>
/// <param name="pWaveFormat">
/// [in] Format of file to write.
/// </param>
/// <param name="dataSize">
/// Number of bytes of data in file's data section.
/// </param>
/// <returns>
/// S_OK on success, otherwise failure code.
/// </returns>
HRESULT WriteWaveHeader(HANDLE waveFile, const WAVEFORMATEX *pWaveFormat, DWORD dataSize)
{
    DWORD waveHeaderSize = sizeof(WAVEHEADER) + sizeof(WAVEFORMATEX) + pWaveFormat->cbSize + sizeof(WaveData) + sizeof(DWORD);
    WAVEHEADER waveHeader;
    DWORD bytesWritten;

    // Update the sizes in the header
    memcpy_s(&waveHeader, sizeof(waveHeader), WaveHeaderTemplate, sizeof(WaveHeaderTemplate));
    waveHeader.dwSize = waveHeaderSize + dataSize - (2 * sizeof(DWORD));
    waveHeader.dwFmtSize = sizeof(WAVEFORMATEX) + pWaveFormat->cbSize;

    // Write the file header
    if (!WriteFile(waveFile, &waveHeader, sizeof(waveHeader), &bytesWritten, NULL))
    {
        return E_FAIL;
    }

    // Write the format
    if (!WriteFile(waveFile, pWaveFormat, sizeof(WAVEFORMATEX) + pWaveFormat->cbSize, &bytesWritten, NULL))
    {
        return E_FAIL;
    }

    // Write the data header
    if (!WriteFile(waveFile, WaveData, sizeof(WaveData), &bytesWritten, NULL))
    {
        return E_FAIL;
    }

    if (!WriteFile(waveFile, &dataSize, sizeof(dataSize), &bytesWritten, NULL))
    {
        return E_FAIL;
    }

    return S_OK;
}

/// <summary>
/// Get an audio device that corresponds to the specified Kinect sensor, if such a device exists.
/// </summary>
/// <param name="pNuiSensor">
/// [in] Kinect sensor for which we'll find a corresponding audio device.
/// </param>
/// <param name="ppDevice">
/// [out] Pointer to hold matching audio device found.
/// </param>
/// <returns>
/// S_OK on success, otherwise failure code.
/// </returns>
HRESULT GetMatchingAudioDevice(INuiSensor *pNuiSensor, IMMDevice **ppDevice)
{
    IMMDeviceEnumerator *pDeviceEnumerator = NULL;
    IMMDeviceCollection *pDdeviceCollection = NULL;
    HRESULT hr = S_OK;

    *ppDevice = NULL;

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDeviceEnumerator));
    if (SUCCEEDED(hr))
    {
        hr = pDeviceEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &pDdeviceCollection);
        if (SUCCEEDED(hr))
        {
            UINT deviceCount;
            hr = pDdeviceCollection->GetCount(&deviceCount);
            if (SUCCEEDED(hr))
            {
                // Iterate through all active audio capture devices looking for one that matches
                // the specified Kinect sensor.
                for (UINT i = 0 ; i < deviceCount; ++i)
                {
                    IMMDevice *pDevice = NULL;

                    hr = pDdeviceCollection->Item(i, &pDevice);
                    if (SUCCEEDED(hr))
                    {
                        wchar_t *pszGlobalId = NULL;
                        hr = GetGlobalId(pDevice, &pszGlobalId);
                        if (SUCCEEDED(hr) && IsMatchingAudioDevice(pNuiSensor, pszGlobalId))
                        {
                            *ppDevice = pDevice;
                            CoTaskMemFree(pszGlobalId);
                            break;
                        }

                        CoTaskMemFree(pszGlobalId);
                    }

                    SafeRelease(pDevice);
                }
            }

            SafeRelease(pDdeviceCollection);
        }

        SafeRelease(pDeviceEnumerator);
    }

    if (SUCCEEDED(hr) && (NULL == *ppDevice))
    {
        // If nothing went wrong but we haven't found a device, return failure
        hr = E_FAIL;
    }

    return hr;
}