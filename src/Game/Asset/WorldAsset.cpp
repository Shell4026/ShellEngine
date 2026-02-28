#include "Asset/WorldAsset.h"
#include "Game/World.h"

#include <cassert>
namespace sh::game
{
	WorldAsset::WorldAsset() : 
		Asset(ASSET_NAME)
	{
	}

	WorldAsset::WorldAsset(const World& world) :
		Asset(ASSET_NAME)
	{
		worldPtr = &world;
		assetUUID = worldPtr->GetUUID();
	}

	void WorldAsset::SetAsset(const core::SObject& obj)
	{
		assert(obj.GetType().IsChildOf(game::World::GetStaticType()));
		if (!obj.GetType().IsChildOf(game::World::GetStaticType()))
			return;
		worldPtr = static_cast<const World*>(&obj);
		assetUUID = worldPtr->GetUUID();
	}

	SH_GAME_API void WorldAsset::ConvertToGameWorldType(bool bConvert)
	{
		bConvertWorldType = bConvert;
	}

	void WorldAsset::SetAssetData() const
	{
		if (!core::IsValid(worldPtr))
			return;

		auto json = worldPtr->Serialize();
		if (json.empty() || json.is_discarded())
			return;

		if(bConvertWorldType)
			json["type"] = game::World::GetStaticType().name.ToString();

		data = core::Json::to_bson(json);
	}

	auto WorldAsset::ParseAssetData() -> bool
	{
		worldData = core::Json::from_bson(data);
		return !worldData.is_discarded();
	}

	auto WorldAsset::GetWorldData() const -> const core::Json&
	{
		return worldData;
	}
	SH_GAME_API auto WorldAsset::ReleaseWorldData() -> core::Json&&
	{
		return std::move(worldData);
	}
}
