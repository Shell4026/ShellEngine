#include "Prefab.h"
#include "World.h"
#include "GameObject.h"

#include "Core/SObjectManager.h"

#include <queue>
namespace sh::game
{
	Prefab::Prefab() :
		rootObjUUID(core::UUID::GenerateEmptyUUID())
	{
	}
	SH_GAME_API auto Prefab::Serialize() const -> core::Json
	{
		core::Json mainJson = Super::Serialize();
		mainJson["rootObj"] = rootObjUUID.ToString();
		mainJson["Prefab"] = prefabJson;
		return mainJson;
	}
	SH_GAME_API void Prefab::Deserialize(const core::Json& json)
	{
		Super::Deserialize(json);
		if (json.contains("rootObj"))
			rootObjUUID = core::UUID{ json["rootObj"].get_ref<const std::string&>() };
		if (json.contains("Prefab"))
			prefabJson = json["Prefab"];
	}
	SH_GAME_API auto Prefab::AddToWorld(World& world) -> GameObject*
	{
		if (prefabJson.is_discarded())
			return nullptr;

		std::vector<core::Json> objJsons{};

		// UUID 추출
		std::vector<std::string> uuidStrs{};
		for (auto& objJson : prefabJson)
		{
			uuidStrs.push_back(objJson["uuid"]); // 게임오브젝트 UUID
			for (auto& compJson : objJson["Components"])
				uuidStrs.push_back(compJson["uuid"]); // 컴포넌트 UUID
		}
		std::unordered_map<std::string, std::string> changedUUID{};
		for (auto& uuidStr : uuidStrs)
			changedUUID[std::move(uuidStr)] = core::UUID::Generate().ToString();
		uuidStrs.clear();

		// UUID 변경후 벡터에 넣기
		for (auto& objJson : prefabJson)
		{
			core::Json changedJson = objJson;
			ChangeUUIDS(changedUUID, changedJson);
			objJsons.push_back(std::move(changedJson));
		}

		// 생성만 하는 과정
		std::vector<std::pair<GameObject*, std::reference_wrapper<core::Json>>> added;
		added.reserve(objJsons.size());
		for (auto& objJson : objJsons)
		{
			if (objJson.contains("GameObject"))
			{
				const auto& gameObjJson = objJson["GameObject"];
				if (gameObjJson.contains("bEditorOnly"))
				{
					if (gameObjJson["bEditorOnly"].get<bool>() == true && world.GetType() == game::World::GetStaticType())
						continue;
				}
			}
			GameObject* const obj = world.AddGameObject(objJson["name"].get<std::string>());
			obj->SetUUID(core::UUID{ objJson["uuid"].get<std::string>() });
			added.push_back({ obj, objJson });

			for (auto& compJson : objJson["Components"])
			{
				const std::string& name{ compJson["name"].get_ref<const std::string&>() };
				const std::string& type{ compJson["type"].get_ref<const std::string&>() };
				core::UUID uuid{ compJson["uuid"].get<std::string>() };
				if (type == "Transform") // 트랜스폼은 게임오브젝트 생성 시 이미 만들어져있다.
				{
					if (obj->transform->GetUUID() != uuid)
						obj->transform->SetUUID(uuid);
					continue;
				}

				auto compType = ComponentModule::GetInstance()->GetComponent(name);
				if (compType == nullptr)
				{
					SH_ERROR_FORMAT("Not found component - {}", type);
					continue;
				}
				Component* const component = compType->Create(*obj);
				component->SetUUID(core::UUID{ uuid });
				obj->AddComponent(component);
			}
		}
		// 역직렬화
		for (auto& [obj, json] : added)
		{
			if (obj != nullptr)
				obj->Deserialize(json.get());
		}
		const std::string& changedRootUUIDStr = changedUUID[rootObjUUID.ToString()];
		auto resultObj = static_cast<GameObject*>(core::SObjectManager::GetInstance()->GetSObject(core::UUID{ changedRootUUIDStr }));
		resultObj->PropagateEnable();

		// Awake 호출
		for (auto& [obj, json] : added)
		{
			if (obj == nullptr)
				continue;
			obj->Awake();
			if (obj->IsActive())
				obj->OnEnable();
		}
		return resultObj;
	}
	SH_GAME_API auto Prefab::operator=(const Prefab& other) -> Prefab&
	{
		rootObjUUID = other.rootObjUUID;
		prefabJson = other.prefabJson;
		return *this;
	}
	SH_GAME_API auto Prefab::operator=(Prefab&& other) noexcept -> Prefab&
	{
		rootObjUUID = other.rootObjUUID;
		prefabJson = std::move(other.prefabJson);
		return *this;
	}
	SH_GAME_API auto Prefab::CreatePrefab(const GameObject& obj) -> Prefab*
	{
		std::string name = obj.GetName().ToString();
		Prefab* prefab = core::SObject::Create<Prefab>();

		std::vector<GameObject*> gameObjects{};

		// BFS로 순회 후 모든 자식 오브젝트를 찾음
		std::queue<Transform*> bfs;
		bfs.push(obj.transform);
		while (!bfs.empty())
		{
			Transform* transform = bfs.front();
			bfs.pop();

			gameObjects.push_back(&transform->gameObject);

			for (auto child : transform->GetChildren())
				bfs.push(child);
		}

		core::Json prefabJson{};
		for (auto obj : gameObjects)
			prefabJson[obj->GetUUID().ToString()] = obj->Serialize();

		prefab->rootObjUUID = obj.GetUUID();
		prefab->prefabJson = std::move(prefabJson);
		prefab->SetName(name);

		return prefab;
	}
	void Prefab::ChangeUUIDS(const std::unordered_map<std::string, std::string>& changed, core::Json& json)
	{
		if (json.is_object())
		{
			for (auto const& [key, val] : json.items())
			{
				ChangeUUIDS(changed, val);
			}
		}
		else if (json.is_array())
		{
			for (auto& item : json)
			{
				ChangeUUIDS(changed, item);
			}
		}
		else if (json.is_string())
		{
			const std::string& value = json.get_ref<const std::string&>();
			if (value.length() != 32)
				return;

			auto it = changed.find(value);
			if (it != changed.end())
				json = it->second;
		}
	}
}