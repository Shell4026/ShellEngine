#include "Asset/PrefabLoader.h"
#include "Asset/PrefabAsset.h"
#include "Prefab.h"

#include "Core/FileSystem.h"
#include "Core/ISerializable.h"
#include "Core/Logger.h"

namespace sh::game
{
	SH_GAME_API PrefabLoader::PrefabLoader()
	{
	}
	SH_GAME_API auto PrefabLoader::Load(const std::filesystem::path& path) const -> core::SObject*
	{
		auto file = core::FileSystem::LoadText(path);
		if (!file.has_value())
			return nullptr;

		const core::Json prefabJson = core::Json::parse(file.value());
		if (!prefabJson.contains("type"))
			return nullptr;
		if (prefabJson["type"].get<std::string>() != "Prefab")
			return nullptr;

		Prefab* prefab = core::SObject::Create<Prefab>();
		prefab->Deserialize(prefabJson);

		return prefab;
	}
	SH_GAME_API auto PrefabLoader::Load(const core::Asset& asset) const -> core::SObject*
	{
		if (std::strcmp(asset.GetType(), PrefabAsset::ASSET_NAME) != 0)
			return nullptr;

		const auto& prefabAsset = static_cast<const PrefabAsset&>(asset);

		const core::Json& prefabJson = prefabAsset.GetPrefabData();
		if (!prefabJson.contains("type"))
			return nullptr;
		if (prefabJson["type"].get<std::string>() != "Prefab")
			return nullptr;

		Prefab* prefab = core::SObject::Create<Prefab>();
		prefab->Deserialize(prefabJson);

		return prefab;
	}
	SH_GAME_API auto PrefabLoader::GetAssetName() const -> const char*
	{
		return PrefabAsset::ASSET_NAME;
	}
}//namespace