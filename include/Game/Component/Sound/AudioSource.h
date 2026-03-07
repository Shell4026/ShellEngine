#pragma once
#include "Game/Export.h"

#include "Game/Component/Component.h"

#include "Sound/SoundClip.h"
#include "Sound/SoundSource.h"

#include <memory>
namespace sh::game
{
	class AudioSource : public Component
	{
		COMPONENT(AudioSource, "Sound")
	public:
		SH_GAME_API AudioSource(GameObject& owner);
		SH_GAME_API ~AudioSource() override;

		SH_GAME_API void Awake() override;
		SH_GAME_API void OnEnable() override;
		SH_GAME_API void Start() override;
		SH_GAME_API void Update() override;
		SH_GAME_API void OnDestroy() override;
		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& prop) override;

		SH_GAME_API void SetClip(sound::SoundClip* clip);

		SH_GAME_API void Play();
		SH_GAME_API void Pause();
		SH_GAME_API void Stop();
		SH_GAME_API void Rewind();

		SH_GAME_API void SetLoop(bool looping);
		SH_GAME_API void SetPlayOnAwake(bool playOnAwake);
		SH_GAME_API void SetVolume(float volume);
		SH_GAME_API void SetPitch(float pitch);
		SH_GAME_API void SetRelative(bool relative);
		SH_GAME_API void SetRolloffFactor(float factor);
		SH_GAME_API void SetReferenceDistance(float dis);
		SH_GAME_API void SetMaxDistance(float dis);

		SH_GAME_API auto IsPlaying() const -> bool;
		SH_GAME_API auto IsLoop() const -> bool { return bLoop; }
		SH_GAME_API auto IsRelative() const -> bool { return bRelative; }
		SH_GAME_API auto IsPlayOnAwake() const -> bool { return bPlayOnAwake; }
		SH_GAME_API auto GetVolume() const -> float { return volume; }
		SH_GAME_API auto GetPitch() const -> float { return pitch; }
		SH_GAME_API auto GetClip() const -> sound::SoundClip* { return clip; }
	private:
		void EnsureSource();
		void ApplyClip();
		void ApplySettings();
		void UpdateSourceTransform();
	private:
		PROPERTY(clip)
		sound::SoundClip* clip = nullptr;
		PROPERTY(volume)
		float volume = 1.f;
		PROPERTY(pitch)
		float pitch = 1.f;
		PROPERTY(rolloffFactor)
		float rolloffFactor = 1.f;
		PROPERTY(referenceDistance)
		float referenceDistance = 1.f;
		PROPERTY(maxDistance)
		float maxDistance = 30.f;
		PROPERTY(bLoop)
		bool bLoop = false;
		PROPERTY(bPlayOnAwake)
		bool bPlayOnAwake = false;
		PROPERTY(bRelative)
		bool bRelative = false;

		std::unique_ptr<sound::SoundSource> source;
	};
}//namespace