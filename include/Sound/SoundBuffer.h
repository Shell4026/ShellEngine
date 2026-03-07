#pragma once
#include "Export.h"

#include "AudioData.h"
#include "Core/NonCopyable.h"

#include <cstddef>
#include <filesystem>
#include <memory>

namespace sh::sound
{
	class SoundSource;

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

		SH_SOUND_API auto GetDuration() const noexcept -> float;
		SH_SOUND_API auto GetFormat() const noexcept -> AudioFormat;
		SH_SOUND_API auto GetSampleRate() const noexcept -> std::uint32_t;
		SH_SOUND_API auto IsValid() const noexcept -> bool;
	private:
		friend class SoundSource;
		SH_SOUND_API auto GetHandle() const noexcept -> unsigned int;
		void Release() noexcept;
	private:
		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}//namespace