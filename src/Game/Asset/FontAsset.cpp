#include "Asset/FontAsset.h"

namespace sh::game
{
	FontAsset::FontAsset() :
		Asset(ASSET_NAME)
	{

	}
	FontAsset::FontAsset(const render::Font& font) :
		Asset(ASSET_NAME)
	{
		fontPtr = &font;
		assetUUID = font.GetUUID();
	}
	SH_GAME_API void FontAsset::SetAsset(const core::SObject& obj)
	{
		if (obj.GetType() != render::Font::GetStaticType())
			return;

		fontPtr = static_cast<const render::Font*>(&obj);
		assetUUID = fontPtr->GetUUID();
	}
	SH_GAME_API void FontAsset::SetAssetData() const
	{
		if (!fontPtr.IsValid())
			return;

		data = core::Json::to_bson(fontPtr->Serialize());
	}
	SH_GAME_API auto FontAsset::ParseAssetData() -> bool
	{
		fontPtr.Reset();

		fontJson = core::Json::from_bson(data);
		return !fontJson.is_discarded();
	}
}//namespace