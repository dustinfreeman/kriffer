//Add to VC++ Directories:
//Include Directories: $(KINECTSDK10_DIR)\inc;
//Library Directories: $(KINECTSDK10_DIR)\lib\x86;

//C/C++ -> Preprocessor -> Preprocessor Definitions: WIN32;_DEBUG;_WINDOWS;%(PreprocessorDefinitions)

//Linker -> Input -> Additional Dependencies:  Kinect10.lib;

#include <riffer.h>
#include <kprocessor.h>

namespace kfr {
	using namespace rfr;

	struct ImgChunk : Chunk { 
		//a struct to hold uncompressed image data, which is not written to disk.
		unsigned char* image;
		unsigned int image_size;
		ImgChunk(std::string _tag_name = NULL_TAG) : Chunk(_tag_name) {}
		~ImgChunk() {
			delete image;
		}

		void assign_image(BYTE _image, int _size) {
			image_size = _size;
			image = new unsigned char[image_size];
			memcpy(image, _image, image_size);
		}
	};

	int get_num_kinects() {
		int iSensorCount = 0;
		HRESULT hr = NuiGetSensorCount(&iSensorCount);
		return iSensorCount;
	}
};



