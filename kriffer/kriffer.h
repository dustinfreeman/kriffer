//Add to VC++ Directories:
//Include Directories: $(KINECTSDK10_DIR)\inc;
//Library Directories: $(KINECTSDK10_DIR)\lib\x86;

//C/C++ -> Preprocessor -> Preprocessor Definitions: WIN32;_DEBUG;_WINDOWS;%(PreprocessorDefinitions)

//Linker -> Input -> Additional Dependencies:  Kinect10.lib;

#include <riffer.h>

#include "kprocessor.h"
#include "k1processor.h"
#include "k2processor.h"
#include "cvprocessor.h"

#pragma once

namespace kfr {
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
			int k2_index = index - get_num_kinect_1s();
			return new kfr::K2Processor(k2_index, folder, filename, _capture_select, overwrite);
		}

		return nullptr;
	}

};