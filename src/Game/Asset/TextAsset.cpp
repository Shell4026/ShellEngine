#include "Asset/TextAsset.h"

namespace sh::game
{
	TextAsset::TextAsset() :
		Asset(ASSET_NAME)
	{

	}
	TextAsset::TextAsset(const TextObject& textObj) :
		Asset(ASSET_NAME)
	{
		textObjPtr = &textObj;
		assetUUID = textObjPtr->GetUUID();
	}
	SH_GAME_API void TextAsset::SetAsset(const core::SObject& obj)
	{
		if (obj.GetType() != TextObject::GetStaticType())
			return;

		textObjPtr = static_cast<const TextObject*>(&obj);
		assetUUID = textObjPtr->GetUUID();
	}
	SH_GAME_API void TextAsset::SetAssetData() const
	{
		if (!textObjPtr.IsValid())
			return;

		std::vector<uint8_t> rawData(textObjPtr->text.size());
		std::memcpy(rawData.data(), textObjPtr->text.data(), rawData.size());

		data = std::move(rawData);
	}
	SH_GAME_API auto TextAsset::ParseAssetData() -> bool
	{
		textObjPtr.Reset();

		return true;
	}
}//namespace