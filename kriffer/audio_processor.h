#include <iostream>

#include "processor.h"

namespace kfr {
	using namespace rfr;
	
	class AudioProcessor: public Processor {
	friend class KrifferTest;
	public:
		virtual void register_tags() { }

		AudioProcessor(std::string _folder,
			std::string _filename,
			bool overwrite = true)
			: Processor(_folder, _filename, overwrite) {}

		virtual bool isOpened() { return true;  }

		virtual std::string update() = 0;

		virtual std::string get_wav_filename() { return ""; }
		virtual void start_audio_index(int audio_index) { std::cout << "empty audioprocessor start_audio_index called. \n"; }
		virtual void stop_audio() { std::cout << "empty kprocessor stop_audio called. \n"; }

		virtual float last_audio_angle() { return _last_audio_angle; }
		virtual void set_running_avg_audio_interval(float seconds) { _running_avg_audio_interval = seconds; }
		virtual float get_running_avg_audio_volume() { return _running_avg_audio_volume; }
		
	protected:
		int capture_select;

		float _last_audio_angle;
		float _running_avg_audio_interval;
		float _running_avg_audio_volume;

		virtual bool ProcessAudio() { std::cout << "empty kprocessor process called. \n"; return false; }
	};
};
