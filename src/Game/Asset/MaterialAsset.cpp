#include "Asset/MaterialAsset.h"

#include "Render/Material.h"

namespace sh::game
{
	MaterialAsset::MaterialAsset() :
		Asset(ASSET_NAME)
	{
	}
	MaterialAsset::MaterialAsset(const render::Material& mat) :
		Asset(ASSET_NAME), 
		matPtr(&mat)
	{
		assetUUID = mat.GetUUID();
	}
	SH_GAME_API void MaterialAsset::SetAsset(const core::SObject& obj)
	{
		if (obj.GetType() != render::Material::GetStaticType())
			return;

		matPtr = static_cast<const render::Material*>(&obj);
		assetUUID = obj.GetUUID();
	}
	SH_GAME_API auto MaterialAsset::GetMaterialData() const -> const core::Json&
	{
		return matData;
	}
	SH_GAME_API void MaterialAsset::SetAssetData() const
	{
		if (!core::IsValid(matPtr))
			return;

		data = core::Json::to_bson(matPtr->Serialize());
	}
	SH_GAME_API auto MaterialAsset::ParseAssetData() -> bool
	{
		matData = core::Json::from_bson(data);

		return !matData.is_discarded();
	}
}//namespace