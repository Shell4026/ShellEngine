#include "GameManager.h"
#include "World.h"
#include "GameObject.h"
#include "ComponentModule.h"
#include "ImGUImpl.h"

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
#include "Core/ThreadSyncManager.h"

#include <algorithm>
namespace sh::game
{
	SH_GAME_API void GameManager::Init(render::Renderer& renderer, ImGUImpl& gui)
	{
		this->renderer = &renderer;
		this->gui = &gui;

		immortalWorld = core::SObject::Create<World>(renderer, gui);
		immortalWorld->SetName("immortalWorld");
		core::GarbageCollection::GetInstance()->SetRootSet(immortalWorld);
	}
	SH_GAME_API auto GameManager::GetRenderer() const -> render::Renderer&
	{
		assert(renderer != nullptr);
		return *renderer;
	}
	SH_GAME_API auto GameManager::GetUIContext() const -> ImGUImpl&
	{
		assert(gui != nullptr);
		return *gui;
	}
	SH_GAME_API void GameManager::Clean()
	{
		auto gc = core::GarbageCollection::GetInstance();

		for (auto& [uuid, world] : worlds)
			world->Destroy();

		immortalWorld->Destroy();
		immortalWorld = nullptr;

		core::GarbageCollection::GetInstance()->Collect();
		core::GarbageCollection::GetInstance()->DestroyPendingKillObjs();

		componentLoader.UnloadPlugin();

		worlds.clear();
		mainWorld.Reset();
	}
	SH_GAME_API auto GameManager::GetMainWorld() const -> World*
	{
		return mainWorld.Get();
	}
	SH_GAME_API auto GameManager::GetWorld(const core::UUID& uuid) const -> World*
	{
		auto it = worlds.find(uuid);
		if (it == worlds.end())
			return nullptr;
		return it->second;
	}
	SH_GAME_API void GameManager::UpdateWorlds(float dt)
	{
		gui->Begin();
		for (auto& [uuid, worldPtr] : worlds)
			worldPtr->Update(dt);
		immortalWorld->Update(dt);
		gui->End();

		for (auto& [uuid, worldPtr] : worlds)
			worldPtr->BeforeSync();

		core::ThreadSyncManager::Sync();

		for (auto& [uuid, worldPtr] : worlds)
			worldPtr->AfterSync();

		while (!afterUpdateTaskQueue.empty())
		{
			auto& func = afterUpdateTaskQueue.front();
			func();
			afterUpdateTaskQueue.pop();
		}
	}
	SH_GAME_API void GameManager::LoadWorld(const core::UUID& uuid, LoadMode mode, bool bPlayWorld)
	{
		auto sobjPtr = core::SObject::GetSObjectUsingResolver(uuid);
		if (!core::IsValid(sobjPtr))
			return;

		if (sobjPtr->GetType().IsChildOf(World::GetStaticType()))
		{
			if (mode == LoadMode::Single)
				loadingSingleWorld = static_cast<World*>(sobjPtr);
			else
				loadingWorldQueue.push(static_cast<World*>(sobjPtr));
		}

		if (!bLoadingWorld)
		{
			afterUpdateTaskQueue.push(
				[&, bPlayWorld]()
				{
					// 불러올 월드가 있다면 불러옴
					if (core::IsValid(loadingSingleWorld))
					{
						for (auto& [uuid, worldPtr] : worlds)
							worldPtr->Clear();
						worlds.clear();

						renderer->Clear();
						gui->ClearDrawData();

						loadingSingleWorld->SetupRenderer();
						loadingSingleWorld->InitResource();
						loadingSingleWorld->LoadWorldPoint();
						if (bPlayWorld)
							loadingSingleWorld->Play();
						loadingSingleWorld->Start();
						
						mainWorld = loadingSingleWorld;

						worlds[loadingSingleWorld->GetUUID()] = loadingSingleWorld;
						loadingSingleWorld = nullptr;
					}
					// additive모드로 불러온 월드들
					while (!loadingWorldQueue.empty())
					{
						World* world = loadingWorldQueue.front();
						loadingWorldQueue.pop();

						core::Json* worldPoint = world->GetWorldPoint();
						if (worldPoint != nullptr)
							world->Deserialize(*worldPoint); // 즉시 역직렬화 해야하므로 LoadWorldPoint()를 쓰지 않음
						if (bPlayWorld)
							world->Play();
						world->Start();

						worlds[world->GetUUID()] = world;
					}
					core::GarbageCollection::GetInstance()->Collect();
					core::ThreadSyncManager::Sync();
					core::GarbageCollection::GetInstance()->DestroyPendingKillObjs();
					bLoadingWorld = false;
				}
			);
			bLoadingWorld = true;
		}
	}
	SH_GAME_API void GameManager::UnloadWorld(const core::UUID& uuid)
	{
		auto it = worlds.find(uuid);
		if (it == worlds.end())
			return;

		it->second->Destroy();
		worlds.erase(it);
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
		if (!json.contains("uuids"))
		{
			SH_ERROR_FORMAT("Not have a 'uuids' key: {}", managerPath.u8string());
			return false;
		}
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
		LoadUserModule("ShellEngineUser");

		if (json.contains("starting"))
		{
			const core::UUID startingWorldUUID{ json["starting"].get<std::string>() };
			auto worldAsset = bundle.LoadAsset(startingWorldUUID);
			if (worldAsset == nullptr)
				return false;

			auto worldLoader = AssetLoaderFactory::GetInstance()->GetLoader(WorldAsset::ASSET_NAME);

			game::World* world = static_cast<game::World*>(worldLoader->Load(*worldAsset));
			if (world == nullptr)
				return false;

			worlds[world->GetUUID()] = world;

			world->SetupRenderer();
			world->InitResource();
			world->LoadWorldPoint();
			world->Play();
			world->Start();
		}
		immortalWorld->Play();
		immortalWorld->Start();

		return true;
	}
	SH_GAME_API auto GameManager::CreateImmortalObject(std::string_view name) -> GameObject&
	{
		return *immortalWorld->AddGameObject(name);
	}
	SH_GAME_API void GameManager::SetImmortalObject(GameObject& obj)
	{
		if (&obj.world != immortalWorld && !obj.IsPendingKill())
		{
			auto objPtr = immortalWorld->AddGameObject(obj.GetName().ToString());
			(*objPtr) = obj;
			obj.Destroy();
		}
	}
	SH_GAME_API void GameManager::ClearImmortalObjects()
	{
		immortalWorld->Clear();
	}
	SH_GAME_API void GameManager::AddAterUpdateTask(const std::function<void()>& fn)
	{
		afterUpdateTaskQueue.push(fn);
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
		auto errorShaderPtr = shaderLoader->Load(*errorShaderAsset);
		if (errorShaderPtr != nullptr)
			defaultAssets.push_back(errorShaderPtr);

		auto errorMateralAsset = bundle.LoadAsset(core::UUID{ "bbc4ef7ec45dce223297a224f8093f10" });
		if (errorMateralAsset == nullptr)
			return;
		auto errorMatPtr = materialLoader->Load(*errorMateralAsset);
		if (errorMatPtr != nullptr)
			defaultAssets.push_back(errorMatPtr);
	}

	SH_GAME_API void GameManager::LoadUserModule(const std::filesystem::path& path)
	{
		componentLoader.LoadPlugin(path);
	}
	SH_GAME_API void GameManager::StartWorlds()
	{
		for (auto& [uuid, worldPtr] : worlds)
		{
			worldPtr->Play();
			worldPtr->Start();
		}
		immortalWorld->Play();
		immortalWorld->Start();
	}
	SH_GAME_API void GameManager::StopWorlds()
	{
		for (auto& [uuid, worldPtr] : worlds)
			worldPtr->Stop();
		immortalWorld->Stop();
	}
}//namespace