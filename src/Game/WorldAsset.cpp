#include "WorldAsset.h"
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

	void WorldAsset::SetAssetData() const
	{
		if (!core::IsValid(worldPtr))
			return;

		if (worldPtr->IsLoaded())
			data = core::Json::to_bson(worldPtr->Serialize());
		else
		{
			const core::Json& worldPoint = worldPtr->GetWorldPoint();
			if (worldPoint.empty() || worldPoint.is_discarded())
				return;
			data = core::Json::to_bson(worldPoint);
		}
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
