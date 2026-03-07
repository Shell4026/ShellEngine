#include "Sound/SoundBuffer.h"

#include "Sound/OggLoader.h"
#include "Sound/SoundSystem.h"
#include "Sound/WavLoader.h"

#include "fmt/format.h"

#include <AL/al.h>
#include <AL/alc.h>

#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace
{
	auto ToAlFormat(sh::sound::AudioFormat format) -> ALenum
	{
		switch (format)
		{
		case sh::sound::AudioFormat::Mono8:
			return AL_FORMAT_MONO8;
		case sh::sound::AudioFormat::Mono16:
			return AL_FORMAT_MONO16;
		case sh::sound::AudioFormat::Stereo8:
			return AL_FORMAT_STEREO8;
		case sh::sound::AudioFormat::Stereo16:
			return AL_FORMAT_STEREO16;
		}
		throw std::runtime_error{ "Unsupported audio format." };
	}

	void ThrowIfAlError(std::string_view operation)
	{
		const ALenum errorCode = alGetError();
		if (errorCode != AL_NO_ERROR)
			throw std::runtime_error{ fmt::format("OpenAL error while {}: {}", operation, static_cast<int>(errorCode)) };
	}

	void EnsureSoundSystemReady()
	{
		if (!sh::sound::SoundSystem::GetInstance()->IsInit())
			throw std::runtime_error{ "SoundSystem is not initialized." };
	}
}

namespace sh::sound
{
	struct SoundBuffer::Impl
	{
		ALuint buffer = 0;
		AudioFormat format = AudioFormat::Mono16;
		std::uint32_t sampleRate = 0;
		float duration = 0.f;
	};

	SH_SOUND_API SoundBuffer::SoundBuffer() :
		impl(std::make_unique<Impl>())
	{
		EnsureSoundSystemReady();
		alGetError();
		alGenBuffers(1, &impl->buffer);
		ThrowIfAlError("creating sound buffer");
	}

	SH_SOUND_API SoundBuffer::SoundBuffer(SoundBuffer&& other) noexcept :
		impl(std::move(other.impl))
	{
	}

	SH_SOUND_API auto SoundBuffer::operator=(SoundBuffer&& other) noexcept -> SoundBuffer&
	{
		if (this == &other)
			return *this;

		Release();
		impl = std::move(other.impl);
		return *this;
	}

	SH_SOUND_API SoundBuffer::~SoundBuffer()
	{
		Release();
	}

	SH_SOUND_API void SoundBuffer::SetData(const AudioData& data)
	{
		SetData(data.format, data.samples.data(), data.samples.size(), data.sampleRate);
	}

	SH_SOUND_API void SoundBuffer::SetData(AudioFormat format, const void* data, std::size_t size, std::uint32_t sampleRate)
	{
		if (data == nullptr || size == 0)
			throw std::invalid_argument{ "Audio data is empty." };
		if (sampleRate == 0)
			throw std::invalid_argument{ "Sample rate must be greater than zero." };
		if (!impl)
			impl = std::make_unique<Impl>();

		impl->format = format;
		impl->sampleRate = sampleRate;

		alBufferData(impl->buffer,
			ToAlFormat(format),
			data,
			static_cast<ALsizei>(size),
			static_cast<ALsizei>(sampleRate));
		ThrowIfAlError("uploading audio data to sound buffer");

		const std::uint32_t channels = sound::GetChannelCount(format);
		const std::uint32_t bitsPerSample = sound::GetBitsPerSample(format);
		const float bytesPerFrame = static_cast<float>(channels * (bitsPerSample / 8));
		impl->duration = static_cast<float>(size) / (static_cast<float>(sampleRate) * bytesPerFrame);
	}

	SH_SOUND_API void SoundBuffer::LoadFromFile(const std::filesystem::path& path)
	{
		std::string ext = path.extension().u8string();
		std::transform(ext.begin(), ext.end(), ext.begin(),
			[](unsigned char ch)
			{
				return static_cast<char>(std::tolower(ch));
			});

		if (ext == ".ogg")
			SetData(OggLoader::Load(path));
		else
			SetData(WavLoader::Load(path));
	}

	SH_SOUND_API auto SoundBuffer::GetDuration() const noexcept -> float
	{
		return impl ? impl->duration : 0.f;
	}

	SH_SOUND_API auto SoundBuffer::GetFormat() const noexcept -> AudioFormat
	{
		return impl ? impl->format : AudioFormat::Mono16;
	}

	SH_SOUND_API auto SoundBuffer::GetSampleRate() const noexcept -> std::uint32_t
	{
		return impl ? impl->sampleRate : 0;
	}

	SH_SOUND_API auto SoundBuffer::IsValid() const noexcept -> bool
	{
		return impl != nullptr && impl->buffer != 0;
	}

	SH_SOUND_API auto SoundBuffer::GetHandle() const noexcept -> unsigned int
	{
		return impl ? impl->buffer : 0;
	}

	void SoundBuffer::Release() noexcept
	{
		if (!impl)
			return;

		if (impl->buffer != 0)
		{
			if (alcGetCurrentContext() != nullptr)
				alDeleteBuffers(1, &impl->buffer);
			impl->buffer = 0;
		}
		impl->sampleRate = 0;
		impl->duration = 0.f;
	}
}//namespace