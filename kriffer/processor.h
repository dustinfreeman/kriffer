#pragma once

#include <riffer.h>

namespace kfr {

	using namespace rfr;

	class Processor {
	public:
		CaptureSession* cs;

		virtual void register_tags() = 0;

		Processor(std::string _filename = "./capture.dat", bool overwrite = true) {
			cs = new CaptureSession(_filename, overwrite);
		}

		~Processor() {
			delete cs;
		}

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

		static char* compress_image(ImgChunk* colourChunk, int* olen) {
			/*unsigned int*/ *olen = colourChunk->image_size*PADDING_FACTOR;
			void* obuf = malloc(*olen);
			//http://code.google.com/p/jpeg-compressor/
			colourChunk->valid_compression = jpge::compress_image_to_jpeg_file_in_memory(obuf, *olen, 
				*colourChunk->get_parameter<int>("width"),
				*colourChunk->get_parameter<int>("height"),
				NUM_CLR_CHANNELS,
				colourChunk->image);
			//Colour format from Kinect:
			//http://msdn.microsoft.com/en-us/library/jj131027.aspx
			//X8R8G8B8
			char* comp_img = new char[*olen];
			memcpy(comp_img, obuf, *olen);
			free(obuf);

			colourChunk->add_parameter("colour image", comp_img, *olen); 
			
			return comp_img;
		}

		virtual std::string update() = 0;

		ImgChunk* get_colour(int64_t ts) {
			ImgChunk* colourChunk = new ImgChunk();
			//std::string tag_filter = tags::get_tag("colour frame");
			cs->get_at_index(colourChunk, "timestamp", ts); //, tag_filter);

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

		virtual void stop() {
			cs->close();
		}
	};
};