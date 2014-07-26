//An Audio Processor that uses SFML.
//starts recording and saves out audio files.

#include "audio_processor.h"

//#include "kfr_sound_buffer_recorder.h"

#include <queue>

namespace kfr {
	using namespace rfr;

	class SFMLAudioProcessor : public AudioProcessor {
	public:
		SFMLAudioProcessor(std::string _folder = RFR_DEFAULT_FOLDER);

		std::string update();

		std::string get_wav_filename();
		void start_audio_index(int audio_index);
		void stop_audio();

	protected:
		sf::SoundBufferRecorder* recorder;
		void reset_buffer();
		//bool recording_audio;
		//int64_t last_reset;
		//float reset_interval_s;

		std::string current_filename;
		std::string get_filename(int index) {
			std::ostringstream ss;
			ss << cs->get_folder();
			ss << "nuiaudio-";
			ss << index;
			ss << ".wav";
			return ss.str();
		}
	};

	SFMLAudioProcessor::SFMLAudioProcessor(std::string _folder)
		: AudioProcessor(_folder, "") {
		current_filename = "";
		
		recorder = nullptr;
		
		//recording_audio = false;
		//reset_buffer();
		//last_reset = get_current_time();
		//reset_interval_s = 60;
	}

	std::string SFMLAudioProcessor::update() {
		//pthread_mutex_lock(&cs_mutex);
		//	_running_avg_audio_volume = recorder->get_running_avg_volume();
		//pthread_mutex_unlock(&cs_mutex);

		//if (!recording_audio) {
		//	//do audio resets so buffer doesn't get too large.
		//	int64_t now = get_current_time();
		//	float since_last_reset_s = (now - last_reset) / std::pow(10.0, 7.0);
		//	if (since_last_reset_s > reset_interval_s) {
		//		reset_buffer();
		//		last_reset = now;
		//	}
		//}

		return "";
	}

	std::string SFMLAudioProcessor::get_wav_filename() {
		return current_filename;
	}

	void SFMLAudioProcessor::reset_buffer() {
		//pthread_mutex_lock(&cs_mutex);
			//clear out recorder
			if (recorder) {
				recorder->stop(); //if not done already.
				delete recorder;
				recorder = nullptr;
			}
		//pthread_mutex_unlock(&cs_mutex);
	}

	void SFMLAudioProcessor::start_audio_index(int audio_index) {
		current_filename = get_filename(audio_index);

		reset_buffer();
		recorder = new sf::SoundBufferRecorder();
		recorder->start();
		//recorder->set_running_avg_interval(this->_running_avg_audio_interval);
	}

	void SFMLAudioProcessor::stop_audio() {
		//pthread_mutex_lock(&cs_mutex);
			recorder->stop();
			recorder->getBuffer().saveToFile(current_filename);
		//pthread_mutex_lock(&cs_mutex);
		
		current_filename = "";
		//recording_audio = false; //allows occasional buffer reset.
		reset_buffer();
		
	}
};

