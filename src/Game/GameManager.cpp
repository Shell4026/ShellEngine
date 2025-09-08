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
namespace sh::game
{
	SH_GAME_API void GameManager::Init(render::Renderer& renderer, ImGUImpl& gui)
	{
		this->renderer = &renderer;
		this->gui = &gui;
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
		for (auto& [uuid, world] : worlds)
		{
			world->Destroy();
		}
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
	SH_GAME_API auto GameManager::GetWorlds() const -> const std::unordered_map<core::UUID, World*>
	{
		return worlds;
	}
	SH_GAME_API void GameManager::UpdateWorlds(float dt)
	{
		for (auto& [uuid, worldPtr] : worlds)
			worldPtr->Update(dt);
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
				[&]()
				{
					// 불러올 월드가 있다면 불러옴
					if (core::IsValid(loadingSingleWorld))
					{
						for (auto& [uuid, worldPtr] : worlds)
							worldPtr->Clean();
						worlds.clear();

						renderer->Clear();
						gui->ClearDrawData();
						gui->AddDrawCallToRenderer();

						loadingSingleWorld->SetRenderPass();
						loadingSingleWorld->InitResource();
						loadingSingleWorld->LoadWorldPoint();
						if (bPlayWorld)
							loadingSingleWorld->Play();
						loadingSingleWorld->Start();

						worlds[loadingSingleWorld->GetUUID()] = loadingSingleWorld;
						mainWorld = loadingSingleWorld;
						loadingSingleWorld = nullptr;
					}
					// additive모드로 불러온 월드들
					while (!loadingWorldQueue.empty())
					{
						World* world = loadingWorldQueue.front();
						loadingWorldQueue.pop();

						world->InitResource();
						world->LoadWorldPoint();
						if (bPlayWorld)
							world->Play();
						world->Start();

						worlds[world->GetUUID()] = world;
					}
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

			world->SetRenderPass();
			world->InitResource();
			world->LoadWorldPoint();
			world->Play();
			world->Start();
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

	SH_GAME_API void GameManager::LoadUserModule(const std::filesystem::path& path, bool bCopy)
	{
		game::ComponentModule* componentModule = game::ComponentModule::GetInstance();
		std::filesystem::path dllPath = path;
#if _WIN32
		if (path.has_extension())
		{
			if (path.extension() != ".dll")
				dllPath = path.parent_path() / std::filesystem::u8path(path.stem().u8string() + ".dll");
		}
		else
			dllPath = std::filesystem::u8path(path.u8string() + ".dll");

		if (!std::filesystem::exists(dllPath))
		{
			SH_INFO_FORMAT("{} not found", dllPath.u8string());
			return;
		}
		if (bCopy)
		{
			auto pdbPath = path.parent_path() / std::filesystem::path(path.stem().u8string() + ".pdb");
			if (std::filesystem::exists(pdbPath))
				std::filesystem::remove(pdbPath);
			std::filesystem::path pluginPath = path.parent_path() / "temp.dll";
			std::filesystem::copy_file(dllPath, pluginPath, std::filesystem::copy_options::overwrite_existing);

			dllPath = std::move(pluginPath);
		}
#elif __linux__
		if (path.has_extension())
		{
			if (path.extension() != ".so")
				dllPath = path.parent_path() / std::filesystem::u8path("lib" + path.stem().u8string() + ".so");
	}
		else
			dllPath = path.parent_path() / std::filesystem::u8path("lib" + path.stem().u8string() + ".so");

		if (!std::filesystem::exists(dllPath))
		{
			SH_INFO_FORMAT("{} not found", dllPath.u8string());
			return;
		}
		if (bCopy)
		{
			std::filesystem::path pluginPath = path.parent_path() / "temp.so";
			std::filesystem::copy_file(dllPath, pluginPath, std::filesystem::copy_options::overwrite_existing);

			dllPath = std::move(pluginPath);
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

			for (const auto& componentInfo : componentModule->GetWaitingComponents())
				userComponents.push_back({ componentInfo.name, &componentInfo.type });

			componentModule->RegisterWaitingComponents();
		}
	}
	SH_GAME_API void GameManager::ReloadUserModule()
	{
		static bool bReloading = false;

		if (!bReloading)
		{
			afterUpdateTaskQueue.push
			(
				[&]()
				{
					std::filesystem::path pluginPath = "ShellEngineUser";
					if (userPlugin != nullptr && userPlugin->handle != nullptr)
					{
						pluginPath = userPlugin->path;
						for (auto& [uuid, worldPtr] : worlds)
						{
							// 1. 월드 현재 상태 저장
							worldPtr->SaveWorldPoint(worldPtr->Serialize());
							// 2. 유저 코드에 있는 컴포넌트와 같은 컴포넌트들을 모두 제거 후 메모리에서 해제
							for (auto obj : worldPtr->GetGameObjects())
							{
								for (auto component : obj->GetComponents())
								{
									if (component == nullptr)
										continue;
									for (auto& userComponent : userComponents)
									{
										if (component->GetType() == *userComponent.second)
											component->Destroy();
									}
								}
							}
							core::GarbageCollection::GetInstance()->Collect();
							core::GarbageCollection::GetInstance()->DestroyPendingKillObjs();
						}
						// 3. DLL 언로드
						userComponents.clear();
						core::ModuleLoader loader{};
						loader.Clean(*userPlugin.get());
						userPlugin.reset();
					}
					// 4. 다시 불러오고 월드 복원
					LoadUserModule(pluginPath);
					for (auto& [uuid, worldPtr] : worlds)
						worldPtr->LoadWorldPoint();

					bReloading = false;
				}
			);
			bReloading = true;
		}
	}
	SH_GAME_API void GameManager::StartWorlds()
	{
		for (auto& [uuid, worldPtr] : worlds)
		{
			worldPtr->Play();
			worldPtr->Start();
		}
	}
	SH_GAME_API void GameManager::StopWorlds()
	{
		for (auto& [uuid, worldPtr] : worlds)
			worldPtr->Stop();
	}
}//namespace