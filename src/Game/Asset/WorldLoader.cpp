#include "Asset/WorldLoader.h"
#include "Asset/WorldAsset.h"
#include "World.h"

#include "Core/FileSystem.h"
#include "Core/Factory.hpp"
#include "Core/Logger.h"

#include <cstring>
namespace sh::game
{
	WorldLoader::WorldLoader()
	{
	}
	auto WorldLoader::Load(const std::filesystem::path& path) const -> core::SObject*
	{
		auto textOpt = core::FileSystem::LoadText(path);
		if (!textOpt.has_value())
			return nullptr;
		if (textOpt.value().empty())
			return nullptr;

		core::Json json = core::Json::parse(textOpt.value());
		if (json.is_discarded())
			return nullptr;
		if (!json.contains("type"))
			return nullptr;

		World* world = core::Factory<World, World*>::GetInstance()->Create(json["type"]);
		if (world == nullptr)
			return nullptr;

		if (json.contains("uuid"))
			world->SetUUID(core::UUID{ json["uuid"].get<std::string>() });
		world->SaveWorldPoint(std::move(json));

		return world;
	}

	auto WorldLoader::Load(const core::Asset& asset) const -> core::SObject*
	{
		if (std::strcmp(asset.GetType(), WorldAsset::ASSET_NAME) != 0)
		{
			SH_ERROR_FORMAT("Asset({}) is not a world!", asset.GetAssetUUID().ToString());
			return nullptr;
		}

		const auto& worldAsset = static_cast<const WorldAsset&>(asset);
		
		const core::Json& worldJson = worldAsset.GetWorldData();
		if (!worldJson.contains("type"))
			return nullptr;

		World* world = core::Factory<World, World*>::GetInstance()->Create(worldJson["type"]);
		if (world == nullptr)
			return nullptr;

		if (worldJson.contains("uuid"))
			world->SetUUID(core::UUID{ worldJson["uuid"].get<std::string>() });
		world->SaveWorldPoint(worldJson);

		return world;
	}

	auto WorldLoader::GetAssetName() const -> const char*
	{
		return WorldAsset::ASSET_NAME;
	}
}
