#include "Asset/TextLoader.h"
#include "Asset/TextAsset.h"

#include "Core/FileSystem.h"
#include "Core/Reflection.hpp"

namespace sh::game
{
	SH_GAME_API TextLoader::TextLoader()
	{
	}
	SH_GAME_API auto TextLoader::Load(const std::filesystem::path& path) -> core::SObject*
	{
		auto file = core::FileSystem::LoadText(path);
		if (!file.has_value())
			return nullptr;

		TextObject* textObj = core::SObject::Create<TextObject>();
		textObj->text = file.value();

		return textObj;
	}
	SH_GAME_API auto TextLoader::Load(const core::Asset& asset) -> core::SObject*
	{
		if (std::strcmp(asset.GetType(), TextAsset::ASSET_NAME) != 0)
			return nullptr;

		const auto& textAsset = static_cast<const TextAsset&>(asset);

		const std::vector<uint8_t>& rawData = textAsset.GetRawData();
		
		TextObject* textObj = core::SObject::Create<TextObject>();
		textObj->text.resize(rawData.size());
		std::memcpy(textObj->text.data(), rawData.data(), rawData.size());

		if (!textObj->SetUUID(asset.GetAssetUUID()))
		{
			TextObject* oldTextObj = core::reflection::Cast<TextObject>(core::SObjectManager::GetInstance()->GetSObject(asset.GetAssetUUID()));
			if (oldTextObj == nullptr)
				return nullptr;

			*oldTextObj = std::move(*textObj);
		}
		return textObj;
	}
	SH_GAME_API auto TextLoader::GetAssetName() const -> const char*
	{
		return TextAsset::ASSET_NAME;
	}
}//namespace