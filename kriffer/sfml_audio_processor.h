//An Audio Processor that uses SFML.
//starts recording and saves out audio files.

#include "audio_processor.h"

namespace kfr {
	using namespace rfr;

	class SFMLAudioProcessor : public AudioProcessor {
	public:
		SFMLAudioProcessor(std::string _folder = RFR_DEFAULT_FOLDER)
			: AudioProcessor(_folder, "") { }

		std::string update();

		float last_audio_angle();

	protected:

	};

	std::string SFMLAudioProcessor::update() {
		
		return "";
	}

	float SFMLAudioProcessor::last_audio_angle() {
		return 0;
	}
};

