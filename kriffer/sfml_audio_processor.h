//An Audio Processor that uses SFML.
//starts recording and saves out audio files.

#include "audio_processor.h"

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

		//live buffer:
		int64_t last_reset;

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

		last_reset = get_current_time();

	}

	std::string SFMLAudioProcessor::update() {
		//will set: _running_avg_audio_volume
		
		return "";

		int64_t now = get_current_time();
		_running_avg_audio_interval = 10; //seconds
		if (now - last_reset > (int64_t)(_running_avg_audio_interval * std::pow(10, 7))) {
			//reset the buffer every interval
			std::cout << now << " buffer reset \n";
			last_reset = now;

			if (recorder) {
				recorder->stop();
				std::cout << "samples: " << recorder->getBuffer().getSampleCount();
				recorder->start();
			}

			reset_buffer();
		}
		

		//HACK
		return "";

		//test if back buffer is full.
		//if (live_buffer.back()->getBuffer().getDuration().asSeconds() >= _running_avg_audio_interval) {
		//	//pops off the front
		//	//HACK NO POPPING

		//	//live_buffer.front()->stop();
		//	//live_buffer.pop(); //should be erasing, hopefully. No memory leaks, plz.
		//	//volume_buffer.pop();

		//	////enqueue empty
		//	//live_buffer.push(new sf::SoundBufferRecorder());
		//	//volume_buffer.push(0);
		//	//live_buffer.back()->start();
		//}

		////live update summarized volume of back buffer
		//const sf::Int16* back_samples = live_buffer.back()->getBuffer().getSamples();
		//int num_samples = live_buffer.back()->getBuffer().getSampleCount();
		//float _sound_level = 0;
		//int s = 0;
		//while (s < num_samples) {
		//	_sound_level += sqrt(abs(back_samples[s]));
		//	s++;
		//}
		//volume_buffer.back() = _sound_level / num_samples;
		//
		////calculate total average volume (over 2 intervals)
		//float dur_front = live_buffer.front()->getBuffer().getDuration().asSeconds();
		//float dur_back = live_buffer.back()->getBuffer().getDuration().asSeconds();
		//if (dur_front == 0 && dur_back == 0) {
		//	_running_avg_audio_volume = 0; //not getting audio
		//}
		//else {
		//	_running_avg_audio_volume = (volume_buffer.front() * dur_front + volume_buffer.back() * dur_back) / (dur_front + dur_back);
		//}
		//std::cout << "_running_avg_audio_volume: " << _running_avg_audio_volume << "\n";

		return "";
	}

	std::string SFMLAudioProcessor::get_wav_filename() {
		return current_filename;
	}

	void SFMLAudioProcessor::reset_buffer() {
		//clear out recorder
		if (recorder) {
			recorder->stop(); //if not done already.
			delete recorder;
		}

		//start new
		recorder = new sf::SoundBufferRecorder();
		recorder->start();
	}

	void SFMLAudioProcessor::start_audio_index(int audio_index) {
		current_filename = get_filename(audio_index);
		reset_buffer();
	}

	void SFMLAudioProcessor::stop_audio() {
		recorder->stop();
		recorder->getBuffer().saveToFile(current_filename);
		current_filename = "";
	}
};

