#include "kprocessor.h"

#include "kr_util.h"
#include <Kinect.h>

namespace kfr {

	int get_num_kinect_2s() {
		//the word is that multiple Kinect 2's might not yet be supported

		HRESULT hr;
		IKinectSensor*	m_pKinectSensor;
		hr = GetDefaultKinectSensor(&m_pKinectSensor);
		if (FAILED(hr))
		{
			return 0;
		}
		return 1;
	}

	class K2Processor: public KProcessor {
	friend class KrifferTest;
	
	static const int DEPTH_FRAME_WIDTH = 512;
	static const int DEPTH_FRAME_HEIGHT = 424;
	//64:53 ratio. Okay....

	public:
		//constructor ==========
		K2Processor(int _k_index, 
			std::string _folder = RFR_DEFAULT_FOLDER, 
			std::string _filename = RFR_DEFAULT_FILENAME, 
			int _capture_select= CAPTURE_ALL,
			bool overwrite = true);

		std::string get_wav_filename() { return "no valid file - k2processor.";  }
		void start_audio_index(int audio_index) {

		}

		void stop_audio() {

		}

		void stop();

	protected:
		// Current Kinect
		IKinectSensor*          m_pKinectSensor;
		// Depth reader
		IDepthFrameReader*      m_pDepthFrameReader;

		bool ProcessColor();
		bool ProcessDepth();
		bool ProcessAudio();
	};

	K2Processor::K2Processor(int _k_index, 
		std::string _folder, 
		std::string _filename, 
		int _capture_select,
		bool overwrite) 
		: KProcessor(_k_index, _folder, _filename, _capture_select, overwrite) {

		HRESULT hr;

		hr = GetDefaultKinectSensor(&m_pKinectSensor);
		if (FAILED(hr))
		{
			std::cout << "failed GetDefaultKinectSensor \n";
			return;
		}

		if (m_pKinectSensor)
		{
			// Initialize the Kinect and get the depth reader
			IDepthFrameSource* pDepthFrameSource = NULL;

			hr = m_pKinectSensor->Open();

			if (SUCCEEDED(hr))
			{
				hr = m_pKinectSensor->get_DepthFrameSource(&pDepthFrameSource);
			}

			if (SUCCEEDED(hr))
			{
				hr = pDepthFrameSource->OpenReader(&m_pDepthFrameReader);
			}

			SafeRelease(pDepthFrameSource);
		}

		if (!m_pKinectSensor || FAILED(hr))
		{
			std::cout << "No ready Kinect 2 found! \n";
			return;
		}
	}

	bool K2Processor::ProcessColor() {
		bool got_frame = false;

		return got_frame;
	}

	bool K2Processor::ProcessDepth() {
		bool got_frame = false;

		IDepthFrame* pDepthFrame = NULL;
		
		HRESULT hr = m_pDepthFrameReader->AcquireLatestFrame(&pDepthFrame);
		if (SUCCEEDED(hr))
		{
			INT64 nTime = 0;
			IFrameDescription* pFrameDescription = NULL;
			int nWidth = 0;
			int nHeight = 0;
			USHORT nDepthMinReliableDistance = 0;
			USHORT nDepthMaxReliableDistance = 0;
			UINT nBufferSize = 0;
			UINT16 *pBuffer = NULL;

			hr = pDepthFrame->get_RelativeTime(&nTime); //can be used for fps stuff.

			if (SUCCEEDED(hr))
			{
				hr = pDepthFrame->get_FrameDescription(&pFrameDescription);
			}
			if (SUCCEEDED(hr))
			{
				hr = pFrameDescription->get_Width(&nWidth);
			}
			if (SUCCEEDED(hr))
			{
				hr = pFrameDescription->get_Height(&nHeight);
			}
			if (SUCCEEDED(hr))
			{
				hr = pDepthFrame->get_DepthMinReliableDistance(&nDepthMinReliableDistance);
			}
			if (SUCCEEDED(hr))
			{
				hr = pDepthFrame->get_DepthMaxReliableDistance(&nDepthMaxReliableDistance);
			}
			if (SUCCEEDED(hr))
			{
				hr = pDepthFrame->AccessUnderlyingBuffer(&nBufferSize, &pBuffer);            
			}
			if (SUCCEEDED(hr))
			{
				//The meat of the processing.
				//ProcessDepth(nTime, pBuffer, nWidth, nHeight, nDepthMinReliableDistance, nDepthMaxReliableDistance);

				ImgChunk* depthChunk = new ImgChunk("depth frame");
				add_current_time(depthChunk);

				depthChunk->add_parameter("width", nWidth);
				depthChunk->add_parameter("height", nHeight);

				//not sure if there is a long-valued kinect timestamp, as with Kinect 1.

				depthChunk->assign_image(pBuffer, (nWidth * nHeight)*sizeof(UINT16));

				// end pixel is start + width*height - 1
				//const UINT16* pBufferEnd = pBuffer + (nWidth * nHeight);
				//while (pBuffer < pBufferEnd)
				//{
				//	USHORT depth = *pBuffer;


				//	//OLD
				//	// To convert to a byte, we're discarding the most-significant
				//	// rather than least-significant bits.
				//	// We're preserving detail, although the intensity will "wrap."
				//	// Values outside the reliable depth range are mapped to 0 (black).

				//	// Note: Using conditionals in this loop could degrade performance.
				//	// Consider using a lookup table instead when writing production code.
				//	/*BYTE intensity = static_cast<BYTE>((depth >= nMinDepth) && (depth <= nMaxDepth) ? (depth % 256) : 0);

				//	pRGBX->rgbRed   = intensity;
				//	pRGBX->rgbGreen = intensity;
				//	pRGBX->rgbBlue  = intensity;

				//	++pRGBX;
				//	*/

				//	++pBuffer;
				//}
			
				add_depth_chunk(depthChunk);

				got_frame = true;
			}

			SafeRelease(pFrameDescription);
		}

		SafeRelease(pDepthFrame);

		return got_frame;
	}

	bool K2Processor::ProcessAudio() {

		return true;
	}

	void K2Processor::stop() {
		// done with depth frame reader
		SafeRelease(m_pDepthFrameReader);

		// close the Kinect Sensor
		if (m_pKinectSensor)
		{
			m_pKinectSensor->Close();
		}
		SafeRelease(m_pKinectSensor);
	}
};