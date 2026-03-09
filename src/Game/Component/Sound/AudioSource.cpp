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
		onClipDestroyListener.SetCallback(
			[this](const core::SObject* clipPtr)
			{
				Stop();
				SetClip(nullptr);
			}
		);
	}

	SH_GAME_API AudioSource::~AudioSource() = default;

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

		CleanupOneShotSources();
		UpdateSourceTransform();
	}

	SH_GAME_API void AudioSource::OnDestroy()
	{
		StopOneShotSources();
		SetClip(nullptr);

		Super::OnDestroy();
	}

	SH_GAME_API void AudioSource::OnPropertyChanged(const core::reflection::Property& prop)
	{
		Super::OnPropertyChanged(prop);

		const auto propName = prop.GetName();
		if (propName == core::Util::ConstexprHash("clip"))
			SetClip(clip);
		else if (propName == core::Util::ConstexprHash("bLoop") ||
			propName == core::Util::ConstexprHash("volume") ||
			propName == core::Util::ConstexprHash("pitch") ||
			propName == core::Util::ConstexprHash("bRelative") ||
			propName == core::Util::ConstexprHash("rolloffFactor") ||
			propName == core::Util::ConstexprHash("referenceDistance") ||
			propName == core::Util::ConstexprHash("maxDistance"))
		{
			ApplySettings();
		}
	}

	SH_GAME_API void AudioSource::SetClip(sound::SoundClip* clip)
	{
		if (this->clip != nullptr)
			this->clip->onDestroy.UnRegister(onClipDestroyListener);

		this->clip = clip;

		if (this->clip != nullptr)
			this->clip->onDestroy.Register(onClipDestroyListener);

		ApplyClip();
	}

	SH_GAME_API void AudioSource::Play()
	{
		CleanupOneShotSources();
		ApplySettings();
		ApplyClip();
		UpdateSourceTransform(source);

		if (!core::IsValid(clip))
			return;

		source.Play();
	}

	SH_GAME_API void AudioSource::PlayOneshot(const sound::SoundClip& clip)
	{
		if (!core::IsValid(&clip))
			return;

		const sound::SoundBuffer* buffer = clip.GetBuffer();
		if (buffer == nullptr || !buffer->IsValid())
			return;

		CleanupOneShotSources();

		auto& pool = *sound::SoundSourcePool::GetInstance();
		const sound::SoundSourcePool::Handle handle = pool.Acquire(&clip);
		sound::SoundSource* oneShotSource = pool.GetSource(handle);
		if (oneShotSource == nullptr)
			return;

		ApplySettings(*oneShotSource, false);
		oneShotSource->SetBuffer(*buffer);
		UpdateSourceTransform(*oneShotSource);
		oneShotSource->Play();

		oneShotSources.push_back(handle);
	}

	SH_GAME_API void AudioSource::Pause()
	{
		source.Pause();

		CleanupOneShotSources();
		auto& pool = *sound::SoundSourcePool::GetInstance();
		for (const auto handle : oneShotSources)
		{
			sound::SoundSource* oneShotSource = pool.GetSource(handle);
			if (oneShotSource != nullptr)
				oneShotSource->Pause();
		}
	}

	SH_GAME_API void AudioSource::Stop()
	{
		source.Stop();
		StopOneShotSources();
	}

	SH_GAME_API void AudioSource::Rewind()
	{
		source.Rewind();

		CleanupOneShotSources();
		auto& pool = *sound::SoundSourcePool::GetInstance();
		for (const auto handle : oneShotSources)
		{
			sound::SoundSource* oneShotSource = pool.GetSource(handle);
			if (oneShotSource != nullptr)
				oneShotSource->Rewind();
		}
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

	SH_GAME_API void AudioSource::SetRolloffFactor(float factor)
	{
		rolloffFactor = factor;
		ApplySettings();
	}

	SH_GAME_API void AudioSource::SetReferenceDistance(float dis)
	{
		referenceDistance = dis;
		ApplySettings();
	}

	SH_GAME_API void AudioSource::SetMaxDistance(float dis)
	{
		maxDistance = dis;
		ApplySettings();
	}

	SH_GAME_API auto AudioSource::IsPlaying() const -> bool
	{
		if (source.GetState() == sound::SoundState::Playing)
			return true;

		const auto& pool = *sound::SoundSourcePool::GetInstance();
		for (const auto handle : oneShotSources)
		{
			const sound::SoundSource* oneShotSource = pool.GetSource(handle);
			if (oneShotSource != nullptr && oneShotSource->GetState() == sound::SoundState::Playing)
				return true;
		}

		return false;
	}

	void AudioSource::ApplyClip()
	{
		if (!core::IsValid(clip))
		{
			source.ClearBuffer();
			return;
		}

		const sound::SoundBuffer* buffer = clip->GetBuffer();
		if (buffer == nullptr || !buffer->IsValid())
		{
			source.ClearBuffer();
			return;
		}

		source.SetBuffer(*buffer);
	}

	void AudioSource::ApplySettings()
	{
		ApplySettings(source, bLoop);

		CleanupOneShotSources();
		auto& pool = *sound::SoundSourcePool::GetInstance();
		for (const auto handle : oneShotSources)
		{
			sound::SoundSource* oneShotSource = pool.GetSource(handle);
			if (oneShotSource != nullptr)
				ApplySettings(*oneShotSource, false);
		}
	}

	void AudioSource::ApplySettings(sound::SoundSource& source, bool looping) const
	{
		source.SetLoop(looping);
		source.SetGain(volume);
		source.SetPitch(pitch);
		source.SetRelative(bRelative);
		source.SetRolloffFactor(rolloffFactor);
		source.SetReferenceDistance(referenceDistance);
		source.SetMaxDistance(maxDistance);
	}

	void AudioSource::UpdateSourceTransform()
	{
		UpdateSourceTransform(source);

		auto& pool = *sound::SoundSourcePool::GetInstance();
		for (const auto handle : oneShotSources)
		{
			sound::SoundSource* oneShotSource = pool.GetSource(handle);
			if (oneShotSource != nullptr)
				UpdateSourceTransform(*oneShotSource);
		}
	}

	void AudioSource::UpdateSourceTransform(sound::SoundSource& source) const
	{
		const glm::vec3& pos = gameObject.transform->GetWorldPosition();
		source.SetPosition(pos.x, pos.y, pos.z);
	}

	void AudioSource::CleanupOneShotSources()
	{
		if (oneShotSources.empty())
			return;

		auto& pool = *sound::SoundSourcePool::GetInstance();
		std::size_t writeIdx = 0;
		for (const auto handle : oneShotSources)
		{
			sound::SoundSource* oneShotSource = pool.GetSource(handle);
			if (oneShotSource == nullptr)
				continue;

			if (oneShotSource->GetState() == sound::SoundState::Stopped)
			{
				pool.Release(handle);
				continue;
			}

			oneShotSources[writeIdx++] = handle;
		}

		oneShotSources.resize(writeIdx);
	}

	void AudioSource::StopOneShotSources()
	{
		if (oneShotSources.empty())
			return;

		auto& pool = *sound::SoundSourcePool::GetInstance();
		for (const auto handle : oneShotSources)
		{
			sound::SoundSource* oneShotSource = pool.GetSource(handle);
			if (oneShotSource != nullptr)
				oneShotSource->Stop();

			pool.Release(handle);
		}

		oneShotSources.clear();
	}
}//namespace
