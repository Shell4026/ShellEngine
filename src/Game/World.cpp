#include "PCH.h"
#include "World.h"

#include "GameObject.h"
#include "Component/Camera.h"

#include "Core/GarbageCollection.h"
#include "Core/Util.h"
#include "Core/SObjectManager.h"

#include <utility>
#include <cstdint>

namespace sh::game
{
	SH_GAME_API World::World(sh::render::Renderer& renderer, const ComponentModule& componentModule) :
		renderer(renderer), componentModule(componentModule),
		
		shaders(renderer), materials(renderer), meshes(renderer), textures(renderer),
		mainCamera(nullptr),
		lightOctree(render::AABB{-1000, -1000, -1000, 1000, 1000, 1000})
	{
		gc = core::GarbageCollection::GetInstance();

		gc->SetRootSet(this);
	}
	SH_GAME_API World::World(World&& other) noexcept :
		renderer(other.renderer), gc(other.gc), componentModule(other.componentModule),
		
		_deltaTime(other._deltaTime), _fixedDeltaTime(other._fixedDeltaTime),
		objs(std::move(other.objs)), addedObjs(std::move(other.addedObjs)),
		shaders(std::move(other.shaders)), materials(std::move(other.materials)), meshes(std::move(other.meshes)), textures(std::move(other.textures)),
		mainCamera(nullptr),
		lightOctree(std::move(other.lightOctree))
	{
		gc->RemoveRootSet(&other);
		gc->SetRootSet(this);
	}
	SH_GAME_API World::~World()
	{
		SH_INFO("~World");
		Clean();
		physWorld.Clean();
	}

	SH_GAME_API void World::Clean()
	{
		meshes.Clean();
		materials.Clean();
		shaders.Clean();
		textures.Clean();

		CleanObjs();
	}
	SH_GAME_API void World::InitResource()
	{
		SH_INFO("Init resource");
	}
	SH_GAME_API void World::CleanObjs()
	{
		for (auto obj : objs)
		{
			if (core::IsValid(obj))
				obj->Destroy();
		}
		objs.clear();
	}

	SH_GAME_API auto World::AddGameObject(std::string_view name) -> GameObject*
	{
		auto obj = Create<GameObject>(*this, std::string{ name });
		objs.insert(obj);
		gc->SetRootSet(obj);
		onGameObjectAdded.Notify(obj);

		return obj;
	}

	SH_GAME_API void World::DestroyGameObject(std::string_view name)
	{
		GameObject* obj = GetGameObject(name);
		if (obj != nullptr)
			DestroyGameObject(*obj);
	}
	SH_GAME_API void World::DestroyGameObject(GameObject& obj)
	{
		obj.Destroy();
		objs.erase(&obj);
	}

	SH_GAME_API auto World::GetGameObject(std::string_view name) const -> GameObject*
	{
		std::string nameStr{ name };
		if (!addedObjs.empty())
		{
			for (auto obj : addedObjs)
			{
				if (!core::IsValid(obj))
					continue;
				if (obj->GetName() == name)
					return obj;
			}
		}

		for (auto obj : objs)
		{
			if (!core::IsValid(obj))
				continue;
			if (obj->GetName() == name)
				return obj;
		}

		return nullptr;
	}

	SH_GAME_API void World::Start()
	{
		for (auto& obj : objs)
		{
			if (!obj->activeSelf)
				continue;
			obj->Awake();
		}
		for (auto& obj : objs)
		{
			if (!obj->activeSelf)
				continue;
			obj->OnEnable();
		}
		for (auto& obj : objs)
		{
			if (!obj->activeSelf)
				continue;
			obj->Start();
		}
	}
	SH_GAME_API void World::Update(float deltaTime)
	{
		_deltaTime = deltaTime;
		if (!startLoop)
			startLoop = true;

		for (auto addedObj : addedObjs)
			objs.insert(addedObj);

		addedObjs.clear();

		for (auto& obj : objs)
		{
			if (!sh::core::IsValid(obj))
				continue;
			if (!obj->activeSelf)
				continue;
			obj->BeginUpdate();
		}
		_fixedDeltaTime += _deltaTime;
		while (_fixedDeltaTime >= FIXED_TIME)
		{
			physWorld.Update(FIXED_TIME);
			for (auto& obj : objs)
			{
				if (!sh::core::IsValid(obj))
					continue;
				if (!obj->activeSelf)
					continue;
				obj->FixedUpdate();
			}
			_fixedDeltaTime -= FIXED_TIME;
		}
		for (auto& obj : objs)
		{
			if (!sh::core::IsValid(obj))
				continue;
			if (!obj->activeSelf)
				continue;
			obj->Update();
		}
		for (auto& obj : objs)
		{
			if (!sh::core::IsValid(obj))
				continue;
			if (!obj->activeSelf)
				continue;
			obj->LateUpdate();
		}
	}

	SH_GAME_API void World::RegisterCamera(Camera* cam)
	{
		if (!core::IsValid(cam))
			return;
		cameras.insert(cam);
		renderer.AddCamera(cam->GetNative());

		onCameraAdd.Notify(cam);
	}
	SH_GAME_API void World::UnRegisterCamera(Camera* cam)
	{
		if (!core::IsValid(cam))
			return;
		cameras.erase(cam);
		renderer.RemoveCamera(cam->GetNative());

		onCameraRemove.Notify(cam);
	}
	SH_GAME_API auto World::GetCameras() const -> const core::SSet<Camera*>&
	{
		return cameras;
	}
	SH_GAME_API void World::SetMainCamera(Camera* cam)
	{
		if (core::IsValid(cam))
		{
			mainCamera = cam;
		}
	}
	SH_GAME_API auto World::GetMainCamera() const -> Camera*
	{
		return mainCamera;
	}

	SH_GAME_API auto World::GetPhysWorld() -> phys::PhysWorld*
	{
		return &physWorld;
	}
	SH_GAME_API auto World::GetLightOctree() -> Octree&
	{
		return lightOctree;
	}
	SH_GAME_API auto World::GetLightOctree() const -> const Octree&
	{
		return lightOctree;
	}

	SH_GAME_API auto World::Serialize() const -> core::Json
	{
		core::Json mainJson{ Super::Serialize() };

		core::Json objsJson = core::Json::array();
		for (auto obj : objs)
		{
			if (obj->GetName() == "_Helper" || obj->GetName() == "Grid" || obj->GetName() == "EditorCamera" || obj->GetName() == "PickingCamera" || obj->GetName() == "_Axis")
				continue;
			core::Json objJson{ obj->Serialize() };
			objsJson.push_back(objJson);
		}
		mainJson["objs"] = objsJson;

		return mainJson;
	}
	SH_GAME_API void World::Deserialize(const core::Json& json)
	{
		if (!json.contains("objs"))
			return;

		core::SObjectManager* objManager = core::SObjectManager::GetInstance();
		// 유효한 참조를 위해 두번 로드하는 과정을 거친다.
		// 생성만 하는 과정
		for (auto& objJson : json["objs"])
		{
			std::string name = objJson["name"].get<std::string>();
			if (name == "_Helper" || name == "Grid" || name == "EditorCamera" || name == "PickingCamera" || name == "_Axis")
				continue;
			auto obj = this->AddGameObject(name);
			obj->SetUUID(core::UUID{ objJson["uuid"].get<std::string>() });

			for (auto& compJson : objJson["Components"])
			{
				std::string name = compJson["name"].get<std::string>();
				std::string type = compJson["type"].get<std::string>();
				std::string uuid = compJson["uuid"].get<std::string>();
				if (type == "Transform") // 트랜스폼은 게임오브젝트 생성 시 이미 만들어져있다.
				{
					obj->transform->SetUUID(core::UUID{ uuid });
					continue;
				}
				auto compType = ComponentModule::GetInstance()->GetComponent(name);
				if (compType == nullptr)
				{
					SH_ERROR_FORMAT("Not found component - {}", type);
					continue;
				}
				Component* component = compType->Create(*obj);
				component->SetUUID(core::UUID{ uuid });
				obj->AddComponent(component);
			}
		}
		// 역 직렬화
		for (auto& objJson : json["objs"])
		{
			GameObject* obj = static_cast<GameObject*>(objManager->GetSObject(core::UUID{ objJson["uuid"].get<std::string>() }));
			obj->Deserialize(objJson);
		}
	}
}