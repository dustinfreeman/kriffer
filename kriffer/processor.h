#pragma once

#include <riffer.h>

#include "lzfx\lzfx.h"

namespace kfr {

	using namespace rfr;

	class Processor {
	protected:
		ImgChunk* _last_colour;

	public:
		CaptureSession* cs;

		virtual void register_tags() = 0;

		Processor(std::string _folder = "./", std::string _filename = "./capture.dat", bool overwrite = true) {
			cs = new CaptureSession(_folder, _filename, overwrite);
		}

		~Processor() {
			delete cs;
		}

		//returns if capture input source is open, not the capture file.
		virtual bool isOpened() = 0;

		static int64_t get_current_time() {
			FILETIME time; GetSystemTimeAsFileTime(&time);
			//Contains a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC).
			
			//conversion:
			LARGE_INTEGER foo;
			int64_t bar;
			foo.HighPart = time.dwHighDateTime;
			foo.LowPart = time.dwLowDateTime;
			bar = foo.QuadPart;

			return bar;
		}

		void add_current_time(Chunk* chunk) {
			chunk->add_parameter("timestamp", get_current_time());
		}

		static char* compress_image(ImgChunk* img_chunk, std::string compress_to_param, int* olen, std::string comp_style = "JPEG") {
			char* comp_img = nullptr;
			
			*olen = img_chunk->image_size*PADDING_FACTOR;
			void* obuf = malloc(*olen);

			if (comp_style == "JPEG") {
				//expect img_chunk->image to be 4-channel
				
				//http://code.google.com/p/jpeg-compressor/
				img_chunk->valid_compression = jpge::compress_image_to_jpeg_file_in_memory(obuf, *olen, 
					*img_chunk->get_parameter<int>("width"),
					*img_chunk->get_parameter<int>("height"),
					NUM_CLR_CHANNELS,
					img_chunk->image);
				//Colour format from Kinect:
				//http://msdn.microsoft.com/en-us/library/jj131027.aspx
				//X8R8G8B8

				if (img_chunk->valid_compression) {
					comp_img = new char[*olen];
					memcpy(comp_img, obuf, *olen);
					img_chunk->add_parameter(compress_to_param, comp_img, *olen); 
				}

				free(obuf);
			}

			if (comp_style == "LZF") {
				unsigned int uolen;
				int result = lzfx_compress(img_chunk->image, img_chunk->image_size, obuf, &uolen);
				*olen = (int)uolen;

				if (result < 0)
					img_chunk->valid_compression = false;
				else
					img_chunk->valid_compression = true;

				if (img_chunk->valid_compression) {
					comp_img = new char[*olen];
					memcpy(comp_img, obuf, *olen);
					img_chunk->add_parameter(compress_to_param, comp_img, *olen); 
				}

				free(obuf);
			}

			return comp_img;
		}

		virtual std::string update() = 0;

		ImgChunk* get_colour(int64_t ts) {
			ImgChunk* colourChunk = new ImgChunk();
			//std::string tag_filter = tags::get_tag("colour frame");
			cs->get_by_index(colourChunk, ts); //, tag_filter);

			//std::cout << "getting " << ts << " returning " << *colourChunk->get_parameter<int64_t>("timestamp") << "\n";

			unsigned int comp_length;
			const unsigned char* comp_data = (unsigned char*)colourChunk->get_parameter_by_tag_as_char_ptr<char*>(tags::get_tag("colour image"), &comp_length); 

			if (comp_data) {
				int w = *colourChunk->get_parameter<int>("width");
				int h = *colourChunk->get_parameter<int>("height");
				int actual_comps;
				unsigned char * uncomp_img = jpgd::decompress_jpeg_image_from_memory(
					comp_data, 
					comp_length,
					&w, &h, &actual_comps, 
					NUM_CLR_CHANNELS);

				if (uncomp_img) {
					colourChunk->assign_image(uncomp_img, w*h*NUM_CLR_CHANNELS*sizeof(BYTE));
					delete[] uncomp_img;
				}
			}

			return colourChunk;
		}

		ImgChunk* last_colour() {
			return _last_colour;
		}

		virtual void stop() {
			cs->close();
		}
	};
};