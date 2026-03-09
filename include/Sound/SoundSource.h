#pragma once
#include "Export.h"

#include "Core/NonCopyable.h"

#include <array>
#include <cstdint>
#include <memory>

namespace sh::sound
{
	class SoundBuffer;

	enum class SoundState
	{
		Initial,
		Playing,
		Paused,
		Stopped
	};

	class SoundSource : public core::INonCopyable
	{
	public:
		SH_SOUND_API SoundSource();
		SH_SOUND_API SoundSource(SoundSource&& other) noexcept;
		SH_SOUND_API auto operator=(SoundSource&& other) noexcept -> SoundSource&;
		SH_SOUND_API ~SoundSource();

		SH_SOUND_API void SetBuffer(const SoundBuffer& buffer);
		SH_SOUND_API void ClearBuffer();

		SH_SOUND_API void Play();
		SH_SOUND_API void Pause();
		SH_SOUND_API void Stop();
		SH_SOUND_API void Rewind();

		SH_SOUND_API void SetLoop(bool looping);
		SH_SOUND_API auto IsLooping() const -> bool;

		SH_SOUND_API void SetGain(float gain);
		SH_SOUND_API auto GetGain() const -> float;
		SH_SOUND_API void SetPitch(float pitch);
		SH_SOUND_API auto GetPitch() const -> float;

		SH_SOUND_API void SetPosition(float x, float y, float z);
		SH_SOUND_API void SetPosition(const std::array<float, 3>& position);
		SH_SOUND_API void SetVelocity(float x, float y, float z);
		SH_SOUND_API void SetVelocity(const std::array<float, 3>& velocity);
		SH_SOUND_API void SetRelative(bool relative);

		SH_SOUND_API void SetReferenceDistance(float dis);
		SH_SOUND_API void SetRolloffFactor(float factor);
		SH_SOUND_API void SetMaxDistance(float dis);

		SH_SOUND_API void SetPlaybackOffset(float seconds);
		SH_SOUND_API auto GetPlaybackOffset() const -> float;

		SH_SOUND_API auto GetState() const -> SoundState;
	private:
		void Release() noexcept;
	private:
		uint32_t handle = 0;
	};
}//namespace
