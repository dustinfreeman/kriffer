#pragma once

#include <riffer.h>

#include <time.h>
#include <windows.h>

#include "pthread\inc\pthread.h"

#include "img_chunk.h"

#include "../../libjpeg-turbo/include/turbojpeg.h"
#include "lzfx\lzfx.h"

namespace kfr {

	using namespace rfr;

	class Processor {
	protected:
		ImgChunk* _last_colour;
	public:
		CaptureSession* cs;
		pthread_mutex_t cs_mutex;
		pthread_t update_thread;
		bool update_thread_running = true;

		virtual void register_tags() = 0;

		Processor(std::string _folder = RFR_DEFAULT_FOLDER, 
			std::string _filename = RFR_DEFAULT_FILENAME, 
			bool overwrite = true)
			: update_thread_running(false) 
		{
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

		int64_t index_ts(int64_t ts) {
			//given a ts that doesn't neccesarily correspond exactly to a chunk,
			//return the timestamp of the closest frame in the index.

			rfr::FileIndexPt<__int64> index_pt = cs->get_index_info(ts);
			return index_pt.value;
		}

		static char* compress_image(ImgChunk* img_chunk, std::string compress_to_param, int* olen, std::string comp_style = "JPEG") {
			img_chunk->valid_compression = false;
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
				unsigned char* _compressedImage = NULL; //!< Memory is allocated by tjCompress2 if _jpegSize == 0
				long unsigned int _jpegSize = *olen;
				tjhandle _jpegCompressor = tjInitCompress();
				int result = tjCompress2(_jpegCompressor, 
					img_chunk->image,
					*img_chunk->get_parameter<int>("width"),
					*img_chunk->get_parameter<int>("width")*NUM_CLR_CHANNELS,
					*img_chunk->get_parameter<int>("height"),
					TJPF_RGBX,
					&_compressedImage,
					&_jpegSize,
					TJSAMP_444,
					95,
					TJFLAG_FASTDCT);
				img_chunk->valid_compression = (result == 0);
				*olen = (int)_jpegSize;

				//assign compressed image if valid
				if (img_chunk->valid_compression) {
					comp_img = new char[*olen];
					memcpy(comp_img, _compressedImage, *olen);
					//memcpy(comp_img, &obuf[0], *olen);
					img_chunk->add_parameter(compress_to_param, comp_img, *olen); 
				}

				tjDestroy(_jpegCompressor);
				tjFree(_compressedImage);
			}

			if (comp_style == "LZF") {
				unsigned int uolen = *olen;
				void *obuf = malloc(uolen);

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

		void start_update_thread();

		ImgChunk* get_colour(int64_t ts, ImgChunk* colourChunk = new ImgChunk()) {
			//std::string tag_filter = tags::get_tag("colour frame");
			cs->get_by_index(colourChunk, ts, "colour frame"); //, tag_filter);

			//std::cout << "getting " << ts << " returning " << *colourChunk->get_parameter<int64_t>("timestamp") << "\n";

			unsigned int comp_length;
			unsigned char* comp_data = (unsigned char*)colourChunk->get_parameter_by_tag_as_char_ptr<char*>(tags::get_tag("colour image"), &comp_length); 

			if (comp_data) {
				int w = *colourChunk->get_parameter<int>("width");
				int h = *colourChunk->get_parameter<int>("height");

				//jpgd method
				//int actual_comps;
				//unsigned char * uncomp_img = jpgd::decompress_jpeg_image_from_memory(
				//	comp_data, 
				//	comp_length,
				//	&w, &h, &actual_comps, 
				//	NUM_CLR_CHANNELS);

				//turbo-jpeg
				//http://stackoverflow.com/questions/9094691/examples-or-tutorials-of-using-libjpeg-turbos-turbojpeg
				unsigned char *uncomp_img = (unsigned char*)malloc(w * h * NUM_CLR_CHANNELS);
				int jpegSubsamp;
				tjhandle _jpegDecompressor = tjInitDecompress();
				//NOTE: we are handling w&h ineffectively here.
				tjDecompressHeader2(_jpegDecompressor, comp_data, comp_length, &w, &h, &jpegSubsamp);
				int result = tjDecompress2(_jpegDecompressor, comp_data, comp_length, 
					uncomp_img, w, w*NUM_CLR_CHANNELS, h, TJPF_RGBX, TJFLAG_FASTDCT);
				tjDestroy(_jpegDecompressor);

				if (result == 0) {
					colourChunk->assign_image(uncomp_img, w*h*NUM_CLR_CHANNELS*sizeof(BYTE));
					
					//cv::Mat uncomp_mat(h, w, CV_8UC4, uncomp_img);
					//cv::imshow("uncomp_mat", uncomp_mat);
				}
				else {
					char* error = tjGetErrorStr();
					std::cout << "libjpeg-turbo error: " << error << "\n";
				}

				free(uncomp_img);
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

	void *update_loop(void *p) {
		Processor *processor = (Processor*)p;
		while (processor->update_thread_running) {
			processor->update();
		}
		return NULL;
	}

	void Processor::start_update_thread() {
		pthread_mutex_init(&cs_mutex, NULL);
		pthread_create(&update_thread, NULL, update_loop, (void*)this);
	}
};

