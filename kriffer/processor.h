#pragma once

#include <riffer.h>

#include <time.h>
#include <windows.h>

#include "img_chunk.h"
#include "lzfx\lzfx.h"
#include "libjpeg-turbo-wrap.h"

namespace kfr {

	using namespace rfr;

	class Processor {
	protected:
		ImgChunk* _last_colour;

	public:
		CaptureSession* cs;

		virtual void register_tags() = 0;

		Processor(std::string _folder = RFR_DEFAULT_FOLDER, std::string _filename = RFR_DEFAULT_FILENAME, bool overwrite = true) {
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

			if (comp_style == "JPEG") {
				//expect img_chunk->image to be 4-channel
				
				std::vector<unsigned char> obuf(*olen);

				//jpge library
				//jpge::params comp_params;
				//comp_params.m_quality = 95;
				//img_chunk->valid_compression = jpge::compress_image_to_jpeg_file_in_memory(&obuf[0], *olen,
				//	*img_chunk->get_parameter<int>("width"),
				//	*img_chunk->get_parameter<int>("height"),
				//	NUM_CLR_CHANNELS,
				//	img_chunk->image,
				//	comp_params);
				
				//OpenCV
				//cv::Mat _img(*img_chunk->get_parameter<int>("height"), *img_chunk->get_parameter<int>("width"), CV_8UC4, img_chunk->image);
				//std::vector<int> comp_params;
				//comp_params.push_back(CV_IMWRITE_JPEG_QUALITY);
				//comp_params.push_back(95);
				//img_chunk->valid_compression = cv::imencode(".jpg", _img, obuf, comp_params);
				//oddly, olen isn't used in opencv!?

				//libjpeg-turbo
				img_chunk->valid_compression = jpeg::compress(&obuf, 
					olen,
					img_chunk->image, 
					*img_chunk->get_parameter<int>("width"),
					*img_chunk->get_parameter<int>("height"),
					NUM_CLR_CHANNELS);

				//assign compressed image if valid
				if (img_chunk->valid_compression) {
					comp_img = new char[*olen];
					memcpy(comp_img, &obuf[0], *olen);
					img_chunk->add_parameter(compress_to_param, comp_img, *olen); 
				}
			}

			if (comp_style == "LZF") {
				void* obuf = malloc(*olen);

				unsigned int uolen;
				int result = lzfx_compress(img_chunk->image, img_chunk->image_size, obuf, &uolen);
				*olen = (int)uolen;
				img_chunk->valid_compression = (result >= 0);

				//assign compressed image if valid
				if (img_chunk->valid_compression) {
					comp_img = new char[*olen];
					memcpy(comp_img, obuf, *olen);
					img_chunk->add_parameter(compress_to_param, comp_img, *olen); 
				}

				free(obuf);
			}

			float comp_ratio = (*olen) / (float)(img_chunk->image_size);
			//std::cout << "compression ratio: " << comp_style << " " << comp_ratio << "\n";

			return comp_img;
		}

		virtual std::string update() = 0;

		ImgChunk* get_colour(int64_t ts) {
			ImgChunk* colourChunk = new ImgChunk();
			//std::string tag_filter = tags::get_tag("colour frame");
			cs->get_by_index(colourChunk, ts, "colour frame"); //, tag_filter);

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