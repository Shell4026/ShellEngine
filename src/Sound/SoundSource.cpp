#include "Sound/SoundSource.h"

#include "Sound/SoundBuffer.h"
#include "Sound/SoundSystem.h"

#include "fmt/format.h"

#include <AL/al.h>
#include <AL/alc.h>

#include <stdexcept>

namespace
{
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

	auto ToSoundState(ALint state) -> sh::sound::SoundState
	{
		switch (state)
		{
		case AL_INITIAL:
			return sh::sound::SoundState::Initial;
		case AL_PLAYING:
			return sh::sound::SoundState::Playing;
		case AL_PAUSED:
			return sh::sound::SoundState::Paused;
		case AL_STOPPED:
			return sh::sound::SoundState::Stopped;
		}
		return sh::sound::SoundState::Stopped;
	}
}

namespace sh::sound
{
	struct SoundSource::Impl
	{
		ALuint source = 0;
	};

	SH_SOUND_API SoundSource::SoundSource() :
		impl(std::make_unique<Impl>())
	{
		EnsureSoundSystemReady();
		alGetError();
		alGenSources(1, &impl->source);
		ThrowIfAlError("creating sound source");
	}

	SH_SOUND_API SoundSource::SoundSource(SoundSource&& other) noexcept :
		impl(std::move(other.impl))
	{
	}

	SH_SOUND_API auto SoundSource::operator=(SoundSource&& other) noexcept -> SoundSource&
	{
		if (this == &other)
			return *this;

		Release();
		impl = std::move(other.impl);
		return *this;
	}

	SH_SOUND_API SoundSource::~SoundSource()
	{
		Release();
	}

	SH_SOUND_API void SoundSource::SetBuffer(const SoundBuffer& buffer)
	{
		alSourcei(impl->source, AL_BUFFER, static_cast<ALint>(buffer.GetHandle()));
		ThrowIfAlError("binding sound buffer to source");
	}

	SH_SOUND_API void SoundSource::ClearBuffer()
	{
		Stop();
		alSourcei(impl->source, AL_BUFFER, 0);
		ThrowIfAlError("clearing sound buffer from source");
	}

	SH_SOUND_API void SoundSource::Play()
	{
		alSourcePlay(impl->source);
		ThrowIfAlError("playing sound source");
	}

	SH_SOUND_API void SoundSource::Pause()
	{
		alSourcePause(impl->source);
		ThrowIfAlError("pausing sound source");
	}

	SH_SOUND_API void SoundSource::Stop()
	{
		alSourceStop(impl->source);
		ThrowIfAlError("stopping sound source");
	}

	SH_SOUND_API void SoundSource::Rewind()
	{
		alSourceRewind(impl->source);
		ThrowIfAlError("rewinding sound source");
	}

	SH_SOUND_API void SoundSource::SetLoop(bool looping)
	{
		alSourcei(impl->source, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
		ThrowIfAlError("setting sound source loop state");
	}

	SH_SOUND_API auto SoundSource::IsLooping() const -> bool
	{
		ALint value = AL_FALSE;
		alGetSourcei(impl->source, AL_LOOPING, &value);
		ThrowIfAlError("querying sound source loop state");
		return value == AL_TRUE;
	}

	SH_SOUND_API void SoundSource::SetGain(float gain)
	{
		if (gain < 0.f)
			throw std::invalid_argument{ "Gain must be greater than or equal to zero." };

		alSourcef(impl->source, AL_GAIN, gain);
		ThrowIfAlError("setting sound source gain");
	}

	SH_SOUND_API auto SoundSource::GetGain() const -> float
	{
		float gain = 0.f;
		alGetSourcef(impl->source, AL_GAIN, &gain);
		ThrowIfAlError("querying sound source gain");
		return gain;
	}

	SH_SOUND_API void SoundSource::SetPitch(float pitch)
	{
		if (pitch <= 0.f)
			throw std::invalid_argument{ "Pitch must be greater than zero." };

		alSourcef(impl->source, AL_PITCH, pitch);
		ThrowIfAlError("setting sound source pitch");
	}

	SH_SOUND_API auto SoundSource::GetPitch() const -> float
	{
		float pitch = 1.f;
		alGetSourcef(impl->source, AL_PITCH, &pitch);
		ThrowIfAlError("querying sound source pitch");
		return pitch;
	}

	SH_SOUND_API void SoundSource::SetPosition(float x, float y, float z)
	{
		alSource3f(impl->source, AL_POSITION, x, y, z);
		ThrowIfAlError("setting sound source position");
	}

	SH_SOUND_API void SoundSource::SetPosition(const std::array<float, 3>& position)
	{
		SetPosition(position[0], position[1], position[2]);
	}

	SH_SOUND_API void SoundSource::SetVelocity(float x, float y, float z)
	{
		alSource3f(impl->source, AL_VELOCITY, x, y, z);
		ThrowIfAlError("setting sound source velocity");
	}

	SH_SOUND_API void SoundSource::SetVelocity(const std::array<float, 3>& velocity)
	{
		SetVelocity(velocity[0], velocity[1], velocity[2]);
	}

	SH_SOUND_API void SoundSource::SetRelative(bool relative)
	{
		alSourcei(impl->source, AL_SOURCE_RELATIVE, relative ? AL_TRUE : AL_FALSE);
		ThrowIfAlError("setting sound source relative mode");
	}

	SH_SOUND_API void SoundSource::SetReferenceDistance(float dis)
	{
		alSourcef(impl->source, AL_REFERENCE_DISTANCE, dis);
		ThrowIfAlError("setting sound source reference distance");
	}

	SH_SOUND_API void SoundSource::SetRolloffFactor(float factor)
	{
		alSourcef(impl->source, AL_ROLLOFF_FACTOR, factor);
		ThrowIfAlError("setting sound source rolloff factor");
	}

	SH_SOUND_API void SoundSource::SetMaxDistance(float dis)
	{
		alSourcef(impl->source, AL_MAX_DISTANCE, 50.0f);
		ThrowIfAlError("setting sound source max distance");
	}

	SH_SOUND_API void SoundSource::SetPlaybackOffset(float seconds)
	{
		if (seconds < 0.f)
			throw std::invalid_argument{ "Playback offset must be greater than or equal to zero." };

		alSourcef(impl->source, AL_SEC_OFFSET, seconds);
		ThrowIfAlError("setting sound source playback offset");
	}

	SH_SOUND_API auto SoundSource::GetPlaybackOffset() const -> float
	{
		float seconds = 0.f;
		alGetSourcef(impl->source, AL_SEC_OFFSET, &seconds);
		ThrowIfAlError("querying sound source playback offset");
		return seconds;
	}

	SH_SOUND_API auto SoundSource::GetState() const -> SoundState
	{
		ALint state = AL_STOPPED;
		alGetSourcei(impl->source, AL_SOURCE_STATE, &state);
		ThrowIfAlError("querying sound source state");
		return ToSoundState(state);
	}

	void SoundSource::Release() noexcept
	{
		if (!impl)
			return;

		if (impl->source != 0)
		{
			if (alcGetCurrentContext() != nullptr)
				alDeleteSources(1, &impl->source);
			impl->source = 0;
		}
	}
}//namespace