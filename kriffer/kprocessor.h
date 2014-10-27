#include <Windows.h>
#include <Winbase.h> //for time functions.

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <tchar.h>

#include <sstream>
#include <streambuf>

#include <stdint.h>

#pragma once
#define KINECT_MAX_USERS 6 
//I'm annoyed I couldn't find the above in NuiApi.h

namespace kfr {
	using namespace rfr;

	class KProcessor : public AudioProcessor {
	friend class KrifferTest;

	public:
		//factory method =============
		static KProcessor* get_kinect(int index, 
			std::string folder = RFR_DEFAULT_FOLDER, 
			std::string filename = RFR_DEFAULT_FILENAME, 
			int _capture_select = CAPTURE_ALL, 
			bool overwrite = true);
		
		//constructor ==========
		KProcessor(int _k_index, 
			std::string _folder = RFR_DEFAULT_FOLDER, 
			std::string _filename = RFR_DEFAULT_FILENAME, 
			int _capture_select= CAPTURE_ALL,
			bool overwrite = true);

		virtual int kinect_version() { return -1; }

		bool isOpened();

		std::string update();

		float get_last_depth_frame_interval();

		ImgChunk* get_depth(int64_t ts, ImgChunk* depthChunk = new ImgChunk());
		ImgChunk* last_depth();

		void stop();
		
	protected:
		bool kinect_opened;
		int capture_select;

		ImgChunk* _last_depth;
		int64_t _last_depth_time;
		int64_t _last_depth_time_interval;
		ImgChunk* _last_colour;

		int k_index;

		void register_tags();

		virtual bool ProcessColor() { std::cout << "empty kprocessor process called. \n"; return false; }

		void add_depth_chunk(ImgChunk* depthChunk);
		virtual bool ProcessDepth() { std::cout << "empty kprocessor process called. \n"; return false; }
		virtual int depth_spp() {
			return 2; //default: one short for depth, another for player id.
		}

		virtual bool ProcessSkeleton() { return false; }
		
		virtual bool ProcessAudio() { std::cout << "empty kprocessor process called. \n"; return false; }
	};

	KProcessor::KProcessor(int _k_index, 
		std::string _folder, 
		std::string _filename, 
		int _capture_select,
		bool overwrite) 
		:	AudioProcessor(_folder, _filename, overwrite),
			capture_select(_capture_select) {
			
		k_index = _k_index;
		//-ve k_index indicates not to open a Kinect. for testing.

		register_tags();

		_last_depth = nullptr;
		_last_colour = nullptr;
		_last_audio_angle = 0;
		cs->index_by("timestamp");

	}

	bool KProcessor::isOpened() {
		return kinect_opened; //currently, only returns state of Kinect on first open.
	}

	void KProcessor::register_tags() {
		//ensures tags are registered with riffer
		tags::register_tag("width", "WDTH", INT_TYPE);
		tags::register_tag("height", "HGHT", INT_TYPE);
		tags::register_tag("timestamp", "MTMP", INT_64_TYPE);
		tags::register_tag("kinect timestamp", "TMSP", INT_64_TYPE);
			
		tags::register_tag("depth frame", "DPTH", CHUNK_TYPE);
		tags::register_tag("depth image", "DPDT", CHAR_PTR_TYPE);
			
		tags::register_tag("colour frame", "CLUR", CHUNK_TYPE);
		tags::register_tag("colour image", "CLRI", CHAR_PTR_TYPE);
	}

	//Methods from KinectExplorer-D2D

	std::string KProcessor::update() {
		//std::ostringstream new_frames;

		if (capture_select & CAPTURE_AUDIO)
			ProcessAudio();
			
		if (capture_select & CAPTURE_COLOUR)
		{
			if (ProcessColor()) {
				//new_frames << "c";
			}
		}

		if (capture_select & CAPTURE_DEPTH)
		{
			if (ProcessDepth()) {
				//new_frames << "d";
			}
		}
		
		if (capture_select & CAPTURE_SKELETON)
		{
			if (ProcessSkeleton()) {
				//new_frames << "c";
			}
		}

		return "";

		//if (new_frames.str().size() == 0)
		//	return ""; //should not have to do this. :(
		//return new_frames.str();
	}
	
	ImgChunk* KProcessor::get_depth(int64_t ts, ImgChunk* depthChunk) {
		pthread_mutex_lock(&cs_mutex);
			cs->get_by_index(depthChunk, ts, "depth frame"); 
		pthread_mutex_unlock(&cs_mutex);
		//In an ugly hack, we don't need to tag-filter, since we are only adding 
		// depth frames anyway! Yay!

		//do the uncompression.
		unsigned int comp_length;
		const unsigned char* comp_data = (unsigned char*)depthChunk->get_parameter_by_tag_as_char_ptr<char*>(tags::get_tag("depth image"), &comp_length); 

		if (comp_data) {
			int w = *depthChunk->get_parameter<int>("width");
			int h = *depthChunk->get_parameter<int>("height");

			unsigned int image_size = w*h*sizeof(short)*depth_spp();
			unsigned int uolen = image_size*PADDING_FACTOR;
			void *uncomp_data = malloc(uolen);
				
			int result = lzfx_decompress(comp_data, comp_length, uncomp_data, &uolen);
			//std::cout << "uncompressed length: " << uolen << std::endl;
				
			depthChunk->valid_compression = (result >= 0);

			if (depthChunk->valid_compression) {
				depthChunk->assign_image((unsigned char*)uncomp_data, image_size);
			}

			free(uncomp_data);
		} else {
			std::cout << "did not get comp_data \n";
		}

		return depthChunk;
	}

	ImgChunk* KProcessor::last_depth() {
		return _last_depth;

		//ImgChunk* __last_depth = nullptr;
		//pthread_mutex_lock(&cs_mutex);
		//	__last_depth = _last_depth;
		//pthread_mutex_unlock(&cs_mutex);
		//return __last_depth;
	}

	float KProcessor::get_last_depth_frame_interval() {
		//returns last depth interval (to be used for FPS) in seconds
		return _last_depth_time_interval / pow(10.0, 7.0);
	}

	void KProcessor::stop() {
		//stop pushing to capture session and wrap up.
		//capture session stays open.
		Processor::stop();
	}

#include "lzfx\lzfx.h"

	void KProcessor::add_depth_chunk(ImgChunk* depthChunk) {

		int olen;
		char* comp_img = Processor::compress_image(depthChunk, "depth image", &olen,  "LZF");
		//std::cout << "olen " << olen << "\n";

		if(depthChunk->valid_compression) {
			pthread_mutex_lock(&cs_mutex);

				if (capturing)
					cs->add(*depthChunk);

				if (_last_depth != nullptr) {
					int64_t *new_last_depth_time = _last_depth->get_parameter<int64_t>("timestamp");
					//sometimes this is null? Not sure what's going on.
					if (new_last_depth_time) {
						_last_depth_time_interval = *new_last_depth_time - _last_depth_time;
						_last_depth_time = *new_last_depth_time;
					}

					delete _last_depth;
					_last_depth == nullptr;
				}

				_last_depth = depthChunk;
			
			pthread_mutex_unlock(&cs_mutex);

		} else {
			std::cout << "Problem with lzf compression. \n";
		}
	}
};
