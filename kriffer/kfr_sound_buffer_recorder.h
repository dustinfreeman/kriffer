//shamelessly barely-different from SoundBufferRecorder

#include <SFML\Audio.hpp>

namespace kfr {

	class SoundBufferRecorder: public sf::SoundBufferRecorder {
	public:

		virtual void set_running_avg_interval(float seconds) { _running_avg_audio_interval = seconds; }
		virtual float get_running_avg_volume() { return _running_avg_audio_volume; }

		//overload:
		bool onProcessSamples(const sf::Int16* samples, std::size_t sampleCount);

	protected:
		float _running_avg_audio_interval;
		float _running_avg_audio_volume;
	};

	bool SoundBufferRecorder::onProcessSamples(const sf::Int16* samples, std::size_t sampleCount) {
		return sf::SoundBufferRecorder::onProcessSamples(samples, sampleCount);
	}

};
