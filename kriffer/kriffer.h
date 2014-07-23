//General inc & lib requirements.
//Add to VC++ Directories:
//Include Directories: $(KINECTSDK10_DIR)\inc;
//Library Directories: $(KINECTSDK10_DIR)\lib\x86;

//C/C++ -> Preprocessor -> Preprocessor Definitions: WIN32;_DEBUG;_WINDOWS;%(PreprocessorDefinitions)

//Linker -> Input -> Additional Dependencies:  Kinect10.lib;

#pragma once

#include <riffer.h>

#include "cvprocessor.h"

#include "sfml_audio_processor.h"

#include "kprocessor.h"
#include "k1processor.h"
#ifndef NO_KINECT2
	#include "k2processor.h"
#endif

namespace kfr {
#ifdef NO_KINECT2
	int get_num_kinect_2s() {	return 0;	}
#endif

	int get_num_kinects() {
		return get_num_kinect_1s() + get_num_kinect_2s();
	}

	KProcessor* KProcessor::get_kinect(int index, std::string folder, std::string filename, int _capture_select, bool overwrite) {
		//indexes Kinect 1s first, then Kinect 2s

		//default, empty processor
		if (index < 0) {
			return new KProcessor(-1);
		}

		if (index < get_num_kinect_1s()) {
			return new kfr::K1Processor(index, folder, filename, _capture_select, overwrite);
		} else if (index < get_num_kinect_1s() + get_num_kinect_2s()) {
#ifndef NO_KINECT2
			int k2_index = index - get_num_kinect_1s();
			return new kfr::K2Processor(k2_index, folder, filename, _capture_select, overwrite);
#endif
		}

		return nullptr;
	}

};