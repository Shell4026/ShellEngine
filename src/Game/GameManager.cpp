#include "GameManager.h"
#include "World.h"
#include "GameObject.h"
#include "Component/Camera.h"

#include "Core/UUID.h"
#include "Core/GarbageCollection.h"
#include "Core/SObjectManager.h"
namespace sh::game
{
	SH_GAME_API void GameManager::Clean()
	{
		for (auto& [uuid, world] : worlds)
		{
			world->Destroy();
		}
		worlds.clear();
		currentWorld = nullptr;
		startingWorld = nullptr;
	}
	SH_GAME_API void GameManager::AddWorld(World& world)
	{
		core::GarbageCollection::GetInstance()->SetRootSet(&world);
		worlds.insert_or_assign(world.GetUUID(), &world);
	}
	SH_GAME_API void GameManager::RemoveWorld(const core::UUID& uuid)
	{
		auto it = worlds.find(uuid);
		if (it == worlds.end())
			return;
		core::GarbageCollection::GetInstance()->RemoveRootSet(it->second);
		worlds.erase(it);
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
		return currentWorld;
	}
	SH_GAME_API void GameManager::SetStartingWorld(World& world)
	{
		startingWorld = &world;
	}
	SH_GAME_API auto GameManager::GetStartingWorld() const -> World*
	{
		return startingWorld;
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
		currentWorld->BeforeSync();
	}
	SH_GAME_API void GameManager::UnloadWorld(World& world)
	{
		world.Clean();
	}
}//namespace