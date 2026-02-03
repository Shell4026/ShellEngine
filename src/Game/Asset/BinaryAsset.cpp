#include "Asset/BinaryAsset.h"

namespace sh::game
{
	BinaryAsset::BinaryAsset() :
		Asset(ASSET_NAME)
	{

	}
	BinaryAsset::BinaryAsset(const BinaryObject& binaryObj) :
		Asset(ASSET_NAME)
	{
		binaryObjPtr = &binaryObj;
		assetUUID = binaryObjPtr->GetUUID();
	}
	SH_GAME_API void BinaryAsset::SetAsset(const core::SObject& obj)
	{
		if (obj.GetType() != BinaryObject::GetStaticType())
			return;

		binaryObjPtr = static_cast<const BinaryObject*>(&obj);
		assetUUID = binaryObjPtr->GetUUID();
	}
	SH_GAME_API void BinaryAsset::SetAssetData() const
	{
		if (!binaryObjPtr.IsValid())
			return;

		data = binaryObjPtr->data;
	}
	SH_GAME_API auto BinaryAsset::ParseAssetData() -> bool
	{
		binaryObjPtr.Reset();

		return true;
	}
}//namespace