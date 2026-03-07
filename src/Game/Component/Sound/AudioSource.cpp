#include "Component/Sound/AudioSource.h"
#include "game/GameObject.h"

#include "Core/Reflection.hpp"
#include "Core/Util.h"

#include "Sound/SoundClip.h"

namespace sh::game
{
	AudioSource::AudioSource(GameObject& owner) :
		Component(owner)
	{
	}

	SH_GAME_API AudioSource::~AudioSource() = default;

	SH_GAME_API void AudioSource::Awake()
	{
		EnsureSource();
	}

	SH_GAME_API void AudioSource::OnEnable()
	{
	}

	SH_GAME_API void AudioSource::Start()
	{
		Super::Start();

		ApplySettings();
		ApplyClip();
		UpdateSourceTransform();

		if (bPlayOnAwake && clip != nullptr)
			Play();
	}

	SH_GAME_API void AudioSource::Update()
	{
		Super::Update();

		if (source == nullptr)
			return;

		UpdateSourceTransform();
	}

	SH_GAME_API void AudioSource::OnDestroy()
	{
		if (source != nullptr)
		{
			source->Stop();
			source.reset();
		}

		Super::OnDestroy();
	}

	SH_GAME_API void AudioSource::OnPropertyChanged(const core::reflection::Property& prop)
	{
		Super::OnPropertyChanged(prop);

		const auto propName = prop.GetName();
		if (propName == core::Util::ConstexprHash("clip"))
			ApplyClip();
		else if (propName == core::Util::ConstexprHash("bLoop") ||
			propName == core::Util::ConstexprHash("volume") ||
			propName == core::Util::ConstexprHash("pitch") ||
			propName == core::Util::ConstexprHash("bRelative"))
		{
			ApplySettings();
		}
	}

	SH_GAME_API void AudioSource::SetClip(sound::SoundClip* clip)
	{
		this->clip = clip;
		ApplyClip();
	}

	SH_GAME_API void AudioSource::Play()
	{
		EnsureSource();
		ApplySettings();
		ApplyClip();
		UpdateSourceTransform();

		if (!core::IsValid(clip))
			return;

		source->Play();
	}

	SH_GAME_API void AudioSource::Pause()
	{
		if (source != nullptr)
			source->Pause();
	}

	SH_GAME_API void AudioSource::Stop()
	{
		if (source != nullptr)
			source->Stop();
	}

	SH_GAME_API void AudioSource::Rewind()
	{
		if (source != nullptr)
			source->Rewind();
	}

	SH_GAME_API void AudioSource::SetLoop(bool looping)
	{
		bLoop = looping;
		ApplySettings();
	}

	SH_GAME_API void AudioSource::SetPlayOnAwake(bool playOnAwake)
	{
		bPlayOnAwake = playOnAwake;
	}

	SH_GAME_API void AudioSource::SetVolume(float volume)
	{
		this->volume = volume;
		ApplySettings();
	}

	SH_GAME_API void AudioSource::SetPitch(float pitch)
	{
		this->pitch = pitch;
		ApplySettings();
	}

	SH_GAME_API void AudioSource::SetRelative(bool relative)
	{
		bRelative = relative;
		ApplySettings();
	}

	SH_GAME_API auto AudioSource::IsPlaying() const -> bool
	{
		return source != nullptr && source->GetState() == sound::SoundState::Playing;
	}

	void AudioSource::EnsureSource()
	{
		if (source == nullptr)
			source = std::make_unique<sound::SoundSource>();
	}

	void AudioSource::ApplyClip()
	{
		if (source == nullptr)
			return;

		if (!core::IsValid(clip))
		{
			source->ClearBuffer();
			return;
		}

		const sound::SoundBuffer* buffer = clip->GetBuffer();
		if (buffer == nullptr || !buffer->IsValid())
		{
			source->ClearBuffer();
			return;
		}

		source->SetBuffer(*buffer);
	}

	void AudioSource::ApplySettings()
	{
		if (source == nullptr)
			return;

		source->SetLoop(bLoop);
		source->SetGain(volume);
		source->SetPitch(pitch);
		source->SetRelative(bRelative);
	}

	void AudioSource::UpdateSourceTransform()
	{
		if (source == nullptr)
			return;

		const glm::vec3& pos = gameObject.transform->GetWorldPosition();
		source->SetPosition(pos.x, pos.y, pos.z);
	}
}//namespace