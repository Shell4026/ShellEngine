#pragma once
#include "Export.h"

#include <cstdint>
#include <vector>

namespace sh::sound
{
	enum class AudioFormat
	{
		Mono8,
		Mono16,
		Stereo8,
		Stereo16
	};

	constexpr auto GetChannelCount(AudioFormat format) noexcept -> std::uint32_t
	{
		switch (format)
		{
		case AudioFormat::Mono8:
		case AudioFormat::Mono16:
			return 1;
		case AudioFormat::Stereo8:
		case AudioFormat::Stereo16:
			return 2;
		}
		return 0;
	}

	constexpr auto GetBitsPerSample(AudioFormat format) noexcept -> std::uint32_t
	{
		switch (format)
		{
		case AudioFormat::Mono8:
		case AudioFormat::Stereo8:
			return 8;
		case AudioFormat::Mono16:
		case AudioFormat::Stereo16:
			return 16;
		}
		return 0;
	}

	struct AudioData
	{
		std::vector<std::uint8_t> samples;
		AudioFormat format = AudioFormat::Mono16;
		std::uint32_t sampleRate = 0;

		auto Empty() const noexcept -> bool
		{
			return samples.empty() || sampleRate == 0;
		}

		auto GetChannelCount() const noexcept -> std::uint32_t
		{
			return sound::GetChannelCount(format);
		}

		auto GetBitsPerSample() const noexcept -> std::uint32_t
		{
			return sound::GetBitsPerSample(format);
		}

		auto GetDuration() const noexcept -> float
		{
			const std::uint32_t channels = GetChannelCount();
			const std::uint32_t bitsPerSample = GetBitsPerSample();
			if (channels == 0 || bitsPerSample == 0 || sampleRate == 0)
				return 0.f;

			const float bytesPerFrame = static_cast<float>(channels * (bitsPerSample / 8));
			if (bytesPerFrame <= 0.f)
				return 0.f;

			return static_cast<float>(samples.size()) / (static_cast<float>(sampleRate) * bytesPerFrame);
		}
	};
}//namespace