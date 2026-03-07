#include "Sound/SoundBuffer.h"

#include "Sound/SoundSystem.h"
#include "Sound/WavLoader.h"

#include "fmt/format.h"

#include <AL/alc.h>

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
	SH_SOUND_API SoundBuffer::SoundBuffer()
	{
		EnsureSoundSystemReady();
		alGetError();
		alGenBuffers(1, &buffer);
		ThrowIfAlError("creating sound buffer");
	}

	SH_SOUND_API SoundBuffer::SoundBuffer(SoundBuffer&& other) noexcept :
		buffer(other.buffer),
		format(other.format),
		sampleRate(other.sampleRate),
		duration(other.duration)
	{
		other.buffer = 0;
		other.sampleRate = 0;
		other.duration = 0.f;
	}

	SH_SOUND_API auto SoundBuffer::operator=(SoundBuffer&& other) noexcept -> SoundBuffer&
	{
		if (this == &other)
			return *this;

		Release();

		buffer = other.buffer;
		format = other.format;
		sampleRate = other.sampleRate;
		duration = other.duration;

		other.buffer = 0;
		other.sampleRate = 0;
		other.duration = 0.f;
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

		this->format = format;
		this->sampleRate = sampleRate;

		alBufferData(buffer,
			ToAlFormat(format),
			data,
			static_cast<ALsizei>(size),
			static_cast<ALsizei>(sampleRate));
		ThrowIfAlError("uploading audio data to sound buffer");

		const std::uint32_t channels = sound::GetChannelCount(format);
		const std::uint32_t bitsPerSample = sound::GetBitsPerSample(format);
		const float bytesPerFrame = static_cast<float>(channels * (bitsPerSample / 8));
		duration = static_cast<float>(size) / (static_cast<float>(sampleRate) * bytesPerFrame);
	}

	SH_SOUND_API void SoundBuffer::LoadFromFile(const std::filesystem::path& path)
	{
		SetData(WavLoader::Load(path));
	}

	void SoundBuffer::Release() noexcept
	{
		if (buffer != 0)
		{
			if (alcGetCurrentContext() != nullptr)
				alDeleteBuffers(1, &buffer);
			buffer = 0;
		}
		sampleRate = 0;
		duration = 0.f;
	}
}
