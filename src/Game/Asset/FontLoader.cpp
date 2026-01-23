#include "Asset/FontLoader.h"
#include "Asset/FontAsset.h"

#include "Core/FileSystem.h"

namespace sh::game
{
	FontLoader::FontLoader()
	{
	}
	SH_GAME_API auto FontLoader::Load(const std::filesystem::path& filePath) -> core::SObject*
	{
		auto file = core::FileSystem::LoadText(filePath);
		if (!file.has_value())
			return nullptr;

		const core::Json fontJson = core::Json::parse(file.value());
		if (!fontJson.contains("type"))
			return nullptr;
		if (fontJson["type"].get_ref<const std::string&>() != "Font")
			return nullptr;

		render::Font* font = core::SObject::Create<render::Font>(render::Font::CreateInfo{});
		font->Deserialize(fontJson);

		return font;
	}
	SH_GAME_API auto FontLoader::Load(const core::Asset& asset) -> core::SObject*
	{
		if (std::strcmp(asset.GetType(), FontAsset::ASSET_NAME) != 0)
			return nullptr;

		const auto& fontAsset = static_cast<const game::FontAsset&>(asset);

		const core::Json& fontJson = fontAsset.GetFontJson();
		if (!fontJson.contains("type"))
			return nullptr;
		if (fontJson["type"].get_ref<const std::string&>() != "Font")
			return nullptr;
		if (!fontJson.contains("uuid"))
			return nullptr;

		const std::string& uuidStr = fontJson["uuid"].get_ref<const std::string&>();

		static core::SObjectManager& sobjManager = *core::SObjectManager::GetInstance();
		auto objPtr = sobjManager.GetSObject(core::UUID{ uuidStr });
		if (objPtr != nullptr)
		{
			render::Font* font = core::reflection::Cast<render::Font>(objPtr);
			assert(font != nullptr);
			if (font == nullptr)
			{
				SH_ERROR_FORMAT("Another object with the same UUID exists! ({})", uuidStr);
				return nullptr;
			}
			font->Deserialize(fontJson);
			return font;
		}

		render::Font* font = core::SObject::Create<render::Font>(render::Font::CreateInfo{});
		font->Deserialize(fontJson);

		return font;
	}
	SH_GAME_API auto FontLoader::GetAssetName() const -> const char*
	{
		return FontAsset::ASSET_NAME;
	}
}//namespace