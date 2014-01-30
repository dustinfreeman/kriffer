#include <riffer.h>

#include "jpg\jpgd.h"
#include "jpg\jpge.h"

#define PADDING_FACTOR 4
#define NUM_CLR_CHANNELS 4

namespace kfr {
	using namespace rfr;

	struct ImgChunk : Chunk { 
		//a struct to hold uncompressed image data, which is not written to disk.
		//makes no assumption about image format.

		unsigned char* image;
		unsigned int image_size;
		bool valid_compression;
		ImgChunk(std::string _tag_name = NULL_TAG) : Chunk(_tag_name) {
			image = nullptr;
			valid_compression = false;
		}
		~ImgChunk() {
			delete[] image;
		}

		void assign_image(BYTE* _image, int _size) {
			//copies the image into local.
			if (image)
				delete image;
			
			image_size = _size;
			image = new unsigned char[image_size];
			memcpy(image, _image, image_size);
		}
	};
}