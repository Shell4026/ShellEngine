#include "Asset/SoundLoader.h"

#include "Asset/SoundAsset.h"

#include "Core/Reflection.hpp"
#include "Core/SObjectManager.h"

#include "Sound/SoundClip.h"

#include <cstring>

namespace sh::game
{
	SH_GAME_API SoundLoader::SoundLoader()
	{
	}

	SH_GAME_API auto SoundLoader::Load(const std::filesystem::path& filePath) const -> core::SObject*
	{
		sound::SoundClip* clip = core::SObject::Create<sound::SoundClip>();
		clip->LoadFromFile(filePath);
		clip->SetName(filePath.stem().u8string());
		return clip;
	}

	SH_GAME_API auto SoundLoader::Load(const core::Asset& asset) const -> core::SObject*
	{
		if (std::strcmp(asset.GetType(), SoundAsset::ASSET_NAME) != 0)
			return nullptr;

		const auto& soundAsset = static_cast<const SoundAsset&>(asset);

		static core::SObjectManager& sobjManager = *core::SObjectManager::GetInstance();
		if (auto objPtr = sobjManager.GetSObject(asset.GetAssetUUID()); objPtr != nullptr)
		{
			sound::SoundClip* clip = core::reflection::Cast<sound::SoundClip>(objPtr);
			if (clip == nullptr)
				return nullptr;

			clip->SetAudioData(soundAsset.GetAudioData());
			return clip;
		}

		sound::SoundClip* clip = core::SObject::Create<sound::SoundClip>();
		clip->SetUUID(asset.GetAssetUUID());
		clip->SetAudioData(soundAsset.GetAudioData());
		return clip;
	}

	SH_GAME_API auto SoundLoader::GetAssetName() const -> const char*
	{
		return SoundAsset::ASSET_NAME;
	}
}//namespace