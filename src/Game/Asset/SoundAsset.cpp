#include "Asset/SoundAsset.h"

#include "Sound/SoundClip.h"

#include <cstring>

namespace sh::game
{
	SoundAsset::SoundAsset() :
		Asset(ASSET_NAME)
	{
	}

	SoundAsset::SoundAsset(const sound::SoundClip& clip) :
		Asset(ASSET_NAME),
		clipPtr(&clip)
	{
		assetUUID = clip.GetUUID();
		audioData = clip.GetAudioData();
	}

	SH_GAME_API void SoundAsset::SetAsset(const core::SObject& obj)
	{
		if (obj.GetType() != sound::SoundClip::GetStaticType())
			return;

		clipPtr = static_cast<const sound::SoundClip*>(&obj);
		assetUUID = clipPtr->GetUUID();
		audioData = clipPtr->GetAudioData();
	}

	SH_GAME_API auto SoundAsset::GetAudioData() const -> const sound::AudioData&
	{
		return audioData;
	}

	SH_GAME_API void SoundAsset::SetAssetData() const
	{
		if (!clipPtr.IsValid())
			return;

		const sound::AudioData& clipAudioData = clipPtr->GetAudioData();
		Header header{};
		header.format = static_cast<std::uint32_t>(clipAudioData.format);
		header.sampleRate = clipAudioData.sampleRate;

		data.resize(sizeof(Header) + clipAudioData.samples.size());
		std::memcpy(data.data(), &header, sizeof(Header));
		if (!clipAudioData.samples.empty())
			std::memcpy(data.data() + sizeof(Header), clipAudioData.samples.data(), clipAudioData.samples.size());
	}

	SH_GAME_API auto SoundAsset::ParseAssetData() -> bool
	{
		clipPtr.Reset();
		audioData = {};

		if (data.size() < sizeof(Header))
			return false;

		Header header{};
		std::memcpy(&header, data.data(), sizeof(Header));

		audioData.format = static_cast<sound::AudioFormat>(header.format);
		audioData.sampleRate = header.sampleRate;
		audioData.samples.resize(data.size() - sizeof(Header));
		if (!audioData.samples.empty())
			std::memcpy(audioData.samples.data(), data.data() + sizeof(Header), audioData.samples.size());
		return true;
	}
}
