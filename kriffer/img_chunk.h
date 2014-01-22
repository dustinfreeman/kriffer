#include <riffer.h>

namespace kfr {
	using namespace rfr;

	struct ImgChunk : Chunk { 
		//a struct to hold uncompressed image data, which is not written to disk.
		unsigned char* image;
		unsigned int image_size;
		ImgChunk(std::string _tag_name = NULL_TAG) : Chunk(_tag_name) {
			image = nullptr;
		}
		~ImgChunk() {
			delete image;
		}

		void assign_image(BYTE* _image, int _size) {
			image_size = _size;
			image = new unsigned char[image_size];
			memcpy(image, _image, image_size);
		}
	};
}