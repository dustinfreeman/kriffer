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
	
	static const int FRAME_WIDTH = 512;
	static const int FRAME_HEIGHT = 424;
	//64:53 ratio. Okay....

	public:
		//constructor ==========
		K2Processor(int _k_index, 
			std::string _folder = RFR_DEFAULT_FOLDER, 
			std::string _filename = RFR_DEFAULT_FILENAME, 
			int _capture_select= CAPTURE_ALL,
			bool overwrite = true);

		std::string get_wav_filename();
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