//Add to VC++ Directories:
//Include Directories: $(KINECTSDK10_DIR)\inc;
//Library Directories: $(KINECTSDK10_DIR)\lib\x86;

//C/C++ -> Preprocessor -> Preprocessor Definitions: WIN32;_DEBUG;_WINDOWS;%(PreprocessorDefinitions)

//Linker -> Input -> Additional Dependencies:  Kinect10.lib;

#include <riffer.h>
#include <kprocessor.h>

namespace kfr {
	using namespace rfr;

	int get_num_kinects() {
		int iSensorCount = 0;
		HRESULT hr = NuiGetSensorCount(&iSensorCount);
		return iSensorCount;
	}
};



