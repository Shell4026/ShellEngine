#include "GameManager.h"
#include "World.h"
#include "ComponentModule.h"

#include "AssetLoaderFactory.h"
#include "Asset/WorldAsset.h"
#include "Asset/ShaderAsset.h"
#include "Asset/MaterialAsset.h"

#include "Core/Logger.h"
#include "Core/UUID.h"
#include "Core/GarbageCollection.h"
#include "Core/SObjectManager.h"
#include "Core/FileSystem.h"
#include "Core/ModuleLoader.h"
namespace sh::game
{
	SH_GAME_API void GameManager::Clean()
	{
		for (auto& [uuid, world] : worlds)
		{
			world->Destroy();
		}
		worlds.clear();

		if (currentWorld != nullptr)
			currentWorld->Destroy();
		currentWorld = nullptr;
		startingWorld = nullptr;
	}
	SH_GAME_API void GameManager::AddWorld(World& world)
	{
		worlds.insert_or_assign(world.GetUUID(), &world);
	}
	SH_GAME_API void GameManager::RemoveWorld(const core::UUID& uuid)
	{
		auto it = worlds.find(uuid);
		if (it == worlds.end())
			return;
		worlds.erase(it);
		worldUUIDs.erase(uuid);
	}
	SH_GAME_API auto GameManager::GetWorld(const core::UUID& uuid) -> World*
	{
		auto it = worlds.find(uuid);
		if (it == worlds.end())
			return nullptr;
		return it->second;
	}
	SH_GAME_API auto GameManager::GetWorlds() const -> const std::unordered_map<core::UUID, World*>
	{
		return worlds;
	}
	SH_GAME_API void GameManager::SetCurrentWorld(World& world)
	{
		currentWorld = &world;
	}
	SH_GAME_API auto GameManager::GetCurrentWorld() const -> World*
	{
		return currentWorld.Get();
	}
	SH_GAME_API void GameManager::SetStartingWorld(World& world)
	{
		worlds.insert_or_assign(world.GetUUID(), &world);
		startingWorld = &world;
	}
	SH_GAME_API auto GameManager::GetStartingWorld() const -> World*
	{
		return startingWorld.Get();
	}
	SH_GAME_API auto GameManager::Serialize() const -> core::Json
	{
		core::Json json{};
		if (startingWorld != nullptr)
			json["starting"] = startingWorld->GetUUID().ToString();

		for (const auto& [uuid, world] : worlds)
			json["worlds"].push_back(world->GetUUID().ToString());

		return json;
	}
	SH_GAME_API void GameManager::Deserialize(const core::Json& json)
	{
		startingWorld = nullptr;
		currentWorld = nullptr;

		for (auto& [uuid, world] : worlds)
			core::GarbageCollection::GetInstance()->RemoveRootSet(world);
		worlds.clear();

		if (json.contains("starting"))
		{
			core::SObject* obj = core::SObjectManager::GetInstance()->GetSObject(core::UUID{ json["starting"].get<std::string>() });
			if (obj != nullptr && obj->GetStaticType() == World::GetStaticType())
				SetStartingWorld(static_cast<World&>(*obj));
			currentWorld = startingWorld;
		}
		if (json.contains("worlds"))
		{
			for (const auto& world : json["worlds"])
			{
				core::SObject* obj = core::SObjectManager::GetInstance()->GetSObject(core::UUID{ world.get<std::string>() });
				if (obj != nullptr && obj->GetStaticType() == World::GetStaticType())
					AddWorld(static_cast<World&>(*obj));
			}
		}
	}
	SH_GAME_API void GameManager::UpdateWorld(float dt)
	{
		if (currentWorld == nullptr)
			return;
		currentWorld->Update(dt);
	}
	SH_GAME_API void GameManager::UnloadWorld(World& world)
	{
		auto it = worlds.find(world.GetUUID());
		if (it != worlds.end())
			worlds.erase(it);
		world.Clean();
	}
	SH_GAME_API auto GameManager::LoadGame(const std::filesystem::path& managerPath, core::AssetBundle& bundle) -> bool
	{
		auto dataOpt = core::FileSystem::LoadBinary(managerPath);
		if (!dataOpt.has_value())
		{
			SH_ERROR_FORMAT("Failed to load gameManager.bin: {}", managerPath.u8string());
			return false;
		}
		const auto json = core::Json::from_bson(dataOpt.value());
		if (json.is_discarded())
		{
			SH_ERROR_FORMAT("Failed to parse gameManager.bin: {}", managerPath.u8string());
			return false;
		}
		if (!json.contains("manager"))
		{
			SH_ERROR_FORMAT("Not have a 'manager' key: {}", managerPath.u8string());
			return false;
		}
		if (!json.contains("uuids"))
		{
			SH_ERROR_FORMAT("Not have a 'uuids' key: {}", managerPath.u8string());
			return false;
		}
		const core::Json& managerJson = json["manager"];
		const core::Json& uuidJson = json["uuids"];
		for (const auto& worldJson : uuidJson)
		{
			for (const auto& [worldUUID, uuids] : worldJson.items())
			{
				for (const auto& uuid : uuids)
					worldUUIDs[core::UUID{ worldUUID }].push_back(core::UUID{ uuid.get<std::string>()});
			}
		}

		LoadDefaultAsset(bundle);
		LoadUserModule();

		if (managerJson.contains("starting"))
		{
			const core::UUID startingWorldUUID{ managerJson["starting"].get<std::string>() };
			auto worldAsset = bundle.LoadAsset(startingWorldUUID);
			if (worldAsset == nullptr)
				return false;

			auto worldLoader = AssetLoaderFactory::GetInstance()->GetLoader(WorldAsset::ASSET_NAME);

			game::World* world = static_cast<game::World*>(worldLoader->Load(*worldAsset));
			if (world == nullptr)
				return false;

			world->LoadWorldPoint();
			world->Play();

			SetStartingWorld(*world);
			SetCurrentWorld(*world);
		}

		return true;
	}
	GameManager::~GameManager()
	{
		Clean();
	}

	void GameManager::LoadDefaultAsset(core::AssetBundle& bundle)
	{
		auto assetLoaderFactory = AssetLoaderFactory::GetInstance();
		auto shaderLoader = assetLoaderFactory->GetLoader(ShaderAsset::ASSET_NAME);
		auto materialLoader = assetLoaderFactory->GetLoader(MaterialAsset::ASSET_NAME);

		auto errorShaderAsset = bundle.LoadAsset(core::UUID{ "bbc4ef7ec45dce223297a224f8093f0f" });
		if (errorShaderAsset == nullptr)
			return;
		shaderLoader->Load(*errorShaderAsset);

		auto errorMateralAsset = bundle.LoadAsset(core::UUID{ "bbc4ef7ec45dce223297a224f8093f10" });
		if (errorMateralAsset == nullptr)
			return;
		materialLoader->Load(*errorMateralAsset);
	}

	void GameManager::LoadUserModule()
	{
		game::ComponentModule* componentModule = game::ComponentModule::GetInstance();
#if _WIN32
		std::filesystem::path dllPath{ "ShellEngineUser.dll" };
		if (!std::filesystem::exists(dllPath))
		{
			SH_INFO("ShellEngineUser.dll not found");
			return;
		}
#elif __linux__
		std::filesystem::path dllPath{ "libShellEngineUser.so" };
		if (!std::filesystem::exists(dllPath))
		{
			SH_INFO("libShellEngineUser.so not found");
			return;
		}
#endif
		core::ModuleLoader loader{};
		auto plugin = loader.Load(dllPath);
		assert(plugin.has_value());
		if (!plugin.has_value())
			SH_ERROR_FORMAT("Can't load module: {}", dllPath.u8string());
		else
		{
			userPlugin = std::make_unique<core::Plugin>(std::move(plugin.value()));

			componentModule->RegisterWaitingComponents();
		}
	}
}//namespace