#pragma once
#include "../Export.h"

#include "Core/Asset.h"
#include "Core/SContainer.hpp"

#include "Sound/AudioData.h"
#include "Sound/SoundClip.h"
namespace sh::game
{
	class SoundAsset : public core::Asset
	{
		SASSET(SoundAsset, "snd")
	public:
		struct Header
		{
			std::uint32_t format = 0;
			std::uint32_t sampleRate = 0;
		};
	public:
		constexpr static const char* ASSET_NAME = "snd";
	public:
		SH_GAME_API SoundAsset();
		SH_GAME_API SoundAsset(const sound::SoundClip& clip);

		SH_GAME_API void SetAsset(const core::SObject& obj) override;
		SH_GAME_API auto GetAudioData() const -> const sound::AudioData&;
	protected:
		SH_GAME_API void SetAssetData() const override;
		SH_GAME_API auto ParseAssetData() -> bool override;
	private:
		core::SObjWeakPtr<const sound::SoundClip> clipPtr;
		sound::AudioData audioData;
	};
}
