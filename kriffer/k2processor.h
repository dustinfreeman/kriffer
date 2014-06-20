#include "kprocessor.h"

namespace kfr {

	class K2Processor: public KProcessor {
	friend class KrifferTest;
	
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
	protected:

		void ProcessColor();
		void ProcessDepth();
		void ProcessAudio();
	};

	K2Processor::K2Processor(int _k_index, 
		std::string _folder, 
		std::string _filename, 
		int _capture_select,
		bool overwrite) 
		: KProcessor(_k_index, _folder, _filename, _capture_select, overwrite) {

	}
};