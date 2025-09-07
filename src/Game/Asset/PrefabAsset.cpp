#include "Asset/PrefabAsset.h"

namespace sh::game
{
	PrefabAsset::PrefabAsset() :
		Asset(ASSET_NAME)
	{
	}
	PrefabAsset::PrefabAsset(const Prefab& prefab) :
		Asset(ASSET_NAME),
		prefabPtr(&prefab)
	{
		assetUUID = prefab.GetUUID();
	}
	SH_GAME_API void PrefabAsset::SetAsset(const core::SObject& obj)
	{
		if (obj.GetType() != Prefab::GetStaticType())
			return;

		prefabPtr = static_cast<const Prefab*>(&obj);
		assetUUID = obj.GetUUID();
	}
	SH_GAME_API auto PrefabAsset::GetPrefabData() const -> const core::Json&
	{
		return prefabData;
	}
	SH_GAME_API void PrefabAsset::SetAssetData() const
	{
		if (!core::IsValid(prefabPtr))
			return;

		data = core::Json::to_bson(prefabPtr->Serialize());
	}
	SH_GAME_API auto PrefabAsset::ParseAssetData() -> bool
	{
		prefabData = core::Json::from_bson(data);

		return !prefabData.is_discarded();
	}
}//namespace
