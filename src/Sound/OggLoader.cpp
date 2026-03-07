#include "Sound/OggLoader.h"

#include "fmt/format.h"

#include "External/stb/stb_vorbis.c"

#include <cstring>
#include <cstdlib>
#include <stdexcept>

namespace sh::sound
{
	SH_SOUND_API auto OggLoader::Load(const std::filesystem::path& path) -> AudioData
	{
		int channels = 0;
		int sampleRate = 0;
		short* decodedSamples = nullptr;

		const int sampleCount = stb_vorbis_decode_filename(path.string().c_str(), &channels, &sampleRate, &decodedSamples);
		if (sampleCount < 0 || decodedSamples == nullptr)
			throw std::runtime_error{ fmt::format("Failed to decode ogg file: {}", path.string()) };

		AudioData audioData{};
		audioData.sampleRate = static_cast<std::uint32_t>(sampleRate);

		if (channels == 1)
			audioData.format = AudioFormat::Mono16;
		else if (channels == 2)
			audioData.format = AudioFormat::Stereo16;
		else
		{
			free(decodedSamples);
			throw std::runtime_error{ fmt::format("Unsupported ogg channel count: {}", channels) };
		}

		const std::size_t byteCount = static_cast<std::size_t>(sampleCount) * static_cast<std::size_t>(channels) * sizeof(short);
		audioData.samples.resize(byteCount);
		std::memcpy(audioData.samples.data(), decodedSamples, byteCount);

		free(decodedSamples);
		return audioData;
	}
}//namespace
