#include <riffer.h>

#include "jpg\jpgd.h"
#include "jpg\jpge.h"

#define PADDING_FACTOR 4
#define NUM_CLR_CHANNELS 4

namespace kfr {
	using namespace rfr;

	struct ImgChunk : Chunk { 
		//a struct to hold uncompressed image data, which is not written to disk.
		unsigned char* image;
		unsigned int image_size;
		bool valid_compression;
		ImgChunk(std::string _tag_name = NULL_TAG) : Chunk(_tag_name) {
			image = nullptr;
			valid_compression = false;
		}
		~ImgChunk() {
			delete image;
		}

		void assign_image(BYTE* _image, int _size) {
			image_size = _size;
			image = new unsigned char[image_size];
			memcpy(image, _image, image_size);

			/*unsigned*/ int olen = image_size*PADDING_FACTOR;
			void* obuf = malloc(olen);
			//http://code.google.com/p/jpeg-compressor/
			bool result = jpge::compress_image_to_jpeg_file_in_memory(obuf, olen, 
				*get_parameter<int>("width"),
				*get_parameter<int>("height"),
				NUM_CLR_CHANNELS,
				image);
			//Colour format from Kinect:
			//http://msdn.microsoft.com/en-us/library/jj131027.aspx
			//X8R8G8B8
			//DOES THIS NEED TO BE CONVERTED FOR JPEG?

			if (!result) {
				std::cout << "Problem with jpge compression. \n";
			} else {
				char* comp_img = new char[olen];
				memcpy(comp_img, obuf, olen);
				add_parameter("colour image", comp_img, olen); 
			}
			free(obuf);

			valid_compression = result;
		}
	};
}