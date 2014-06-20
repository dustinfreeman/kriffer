#include "kprocessor.h"

#define KINECT_MAX_USERS 6 
//I'm annoyed I couldn't find the above in NuiApi.h

namespace kfr {

	class K1Processor: public KProcessor {
	public:

		//constructor ==========
		K1Processor(int _k_index, 
			std::string _folder = RFR_DEFAULT_FOLDER, 
			std::string _filename = RFR_DEFAULT_FILENAME, 
			int _capture_select= CAPTURE_ALL,
			bool overwrite = true);
	};

	K1Processor::K1Processor(int _k_index, 
		std::string _folder, 
		std::string _filename, 
		int _capture_select,
		bool overwrite) 
		: KProcessor(_k_index, _folder, _filename, _capture_select, overwrite) {

	}

};