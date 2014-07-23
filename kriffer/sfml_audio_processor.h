//An Audio Processor that uses SFML.
//starts recording and saves out audio files.

#include "audio_processor.h"

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
	}

	std::string SFMLAudioProcessor::update() {
		//TODO: do live volume stuff...
		// _running_avg_audio_volume




		return "";
	}

	std::string SFMLAudioProcessor::get_wav_filename() {
		return current_filename;
	}

	void SFMLAudioProcessor::start_audio_index(int audio_index) {
		//clear out recorder
		if (recorder)
			delete recorder;

		//start new
		current_filename = get_filename(audio_index);
		recorder = new sf::SoundBufferRecorder();
		recorder->start();
	}

	void SFMLAudioProcessor::stop_audio() {
		recorder->stop();
		recorder->getBuffer().saveToFile(current_filename);
		current_filename = "";
	}
};

