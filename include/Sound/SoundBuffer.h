#pragma once
#include "Export.h"

#include "AudioData.h"
#include "Core/NonCopyable.h"

#include <AL/al.h>

#include <cstddef>
#include <filesystem>

namespace sh::sound
{
	class SoundBuffer : public core::INonCopyable
	{
	public:
		SH_SOUND_API SoundBuffer();
		SH_SOUND_API SoundBuffer(SoundBuffer&& other) noexcept;
		SH_SOUND_API auto operator=(SoundBuffer&& other) noexcept -> SoundBuffer&;
		SH_SOUND_API ~SoundBuffer();

		SH_SOUND_API void SetData(const AudioData& data);
		SH_SOUND_API void SetData(AudioFormat format, const void* data, std::size_t size, std::uint32_t sampleRate);
		SH_SOUND_API void LoadFromFile(const std::filesystem::path& path);

		SH_SOUND_API auto GetHandle() const noexcept -> ALuint { return buffer; }
		SH_SOUND_API auto GetDuration() const noexcept -> float { return duration; }
		SH_SOUND_API auto GetFormat() const noexcept -> AudioFormat { return format; }
		SH_SOUND_API auto GetSampleRate() const noexcept -> std::uint32_t { return sampleRate; }
		SH_SOUND_API auto IsValid() const noexcept -> bool { return buffer != 0; }
	private:
		void Release() noexcept;
	private:
		ALuint buffer = 0;
		AudioFormat format = AudioFormat::Mono16;
		std::uint32_t sampleRate = 0;
		float duration = 0.f;
	};
}
