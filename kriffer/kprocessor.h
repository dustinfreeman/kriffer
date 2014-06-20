#include <Windows.h>
#include <Winbase.h> //for time functions.

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <tchar.h>

#include <sstream>
#include <streambuf>

#include <stdint.h>

#include "img_chunk.h"

#include "processor.h"
#include "lzfx\lzfx.h"

#pragma once
#define KINECT_MAX_USERS 6 
//I'm annoyed I couldn't find the above in NuiApi.h

namespace kfr {
	using namespace rfr;

	const int CAPTURE_DEPTH = 1;
	const int CAPTURE_COLOUR = 2;
	const int CAPTURE_SKELETON = 4;
	const int CAPTURE_AUDIO = 8;
	const int CAPTURE_ALL = CAPTURE_DEPTH + CAPTURE_COLOUR + CAPTURE_SKELETON + CAPTURE_AUDIO;
	
	//below function used to catch audio errors.
	void SignalHandler(int signal)
	{
		printf("Signal %d",signal);
		throw "!Access Violation!";
	}
	//typedef void (*SignalHandlerPointer)(int);

	class KProcessor: public Processor {
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

		bool isOpened();

		std::string update();

		ImgChunk* get_depth(int64_t ts);
		ImgChunk* last_depth();

		std::string get_wav_filename();
		float last_audio_angle();

		void start_audio_index(int audio_index);
		void stop_audio();

		void stop();
		
	protected:
		bool kinect_opened;
		int capture_select;

		ImgChunk* _last_depth;
		ImgChunk* _last_colour;

		float _last_audio_angle;

		int k_index;

		void register_tags();

		virtual void ProcessColor() { std::cout << "empty kprocessor process called. \n"; }

		virtual void ProcessDepth() { std::cout << "empty kprocessor process called. \n"; }

		virtual void ProcessAudio() { std::cout << "empty kprocessor process called. \n"; }
	};

	KProcessor::KProcessor(int _k_index, 
		std::string _folder, 
		std::string _filename, 
		int _capture_select,
		bool overwrite) 
		:	Processor(_folder, _filename, overwrite),
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
		std::ostringstream new_frames;

		if (capture_select & CAPTURE_AUDIO)
			ProcessAudio();
			
		if (capture_select & CAPTURE_COLOUR)
		{
			ProcessColor();
			new_frames << "c";
		}

		// TO INDEX BETWEEN MULTIPLE FRAME TYPES,
		//	we will have to adjust indexing so it can filter a specific frame type.
		if (capture_select & CAPTURE_DEPTH)
		{
			ProcessDepth();
			new_frames << "d";
		}
		/*
		if (WAIT_OBJECT_0 == WaitForSingleObject(skeleton_stream->frameEvent, 0))
		{
			//TODO ProcessSkeleton();
			new_frames << "s";
		}*/
		return new_frames.str();
	}
	
	ImgChunk* KProcessor::get_depth(int64_t ts) {
		ImgChunk* depthChunk = new ImgChunk();
		cs->get_by_index(depthChunk, ts, "depth frame"); 
		//In an ugly hack, we don't need to tag-filter, since we are only adding 
		// depth frames anyway! Yay!

		//std::cout << depthChunk->params.size() << std::endl;

		//do the uncompression.
		unsigned int comp_length;
		const unsigned char* comp_data = (unsigned char*)depthChunk->get_parameter_by_tag_as_char_ptr<char*>(tags::get_tag("depth image"), &comp_length); 

		if (comp_data) {
			int w = *depthChunk->get_parameter<int>("width");
			int h = *depthChunk->get_parameter<int>("height");

			unsigned int image_size = w*h*sizeof(short)*2;
			unsigned int uolen = image_size*PADDING_FACTOR;
			void * uncomp_data = malloc(uolen);
				
			int result = lzfx_decompress(comp_data, comp_length, uncomp_data, &uolen);
			//std::cout << "uncompressed lenght: " << uolen << std::endl;
				
			if (result < 0)
				depthChunk->valid_compression = false;
			else
				depthChunk->valid_compression = true;

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
	}

	float KProcessor::last_audio_angle() {
		return _last_audio_angle;
	}

	void KProcessor::stop() {
		//stop pushing to capture session and wrap up.
		//capture session stays open.
		Processor::stop();
	}
};