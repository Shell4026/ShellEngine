﻿#include "World.h"
#include "GameObject.h"
#include "ImGUImpl.h"
#include "Component/Camera.h"

#include "Core/GarbageCollection.h"
#include "Core/Util.h"
#include "Core/SObjectManager.h"

#include <utility>
#include <cstdint>

namespace sh::game
{
	SH_GAME_API World::World(sh::render::Renderer& renderer, const ComponentModule& componentModule, ImGUImpl& guiContext) :
		renderer(renderer), componentModule(componentModule), imgui(&guiContext),
		
		shaders(renderer), materials(renderer), textures(renderer), models(renderer),
		mainCamera(nullptr),
		lightOctree(render::AABB{-1000, -1000, -1000, 1000, 1000, 1000})
	{
		SetName("World");
		gc = core::GarbageCollection::GetInstance();

		gc->SetRootSet(this);
	}
	SH_GAME_API World::World(World&& other) noexcept :
		renderer(other.renderer), gc(other.gc), componentModule(other.componentModule), imgui(other.imgui),
		
		_deltaTime(other._deltaTime), _fixedDeltaTime(other._fixedDeltaTime),
		objs(std::move(other.objs)), addedObjs(std::move(other.addedObjs)),
		shaders(std::move(other.shaders)), materials(std::move(other.materials)), textures(std::move(other.textures)), models(std::move(other.models)),
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
		models.Clean();
		materials.Clean();
		shaders.Clean();
		textures.Clean();

		CleanObjs();
	}
	SH_GAME_API void World::InitResource()
	{
		SH_INFO("Init resource");

		render::RenderTexture* gameViewTexture = core::SObject::Create<render::RenderTexture>(render::Texture::TextureFormat::SRGBA32);
		gameViewTexture->Build(*renderer.GetContext());
		textures.AddResource("GameView", gameViewTexture);
	}
	auto World::AllocateGameObject() -> GameObject*
	{
		while (!deallocatedObjs.empty())
		{
			objPool.DeAllocate(deallocatedObjs.front());
			deallocatedObjs.pop();
		}
		return objPool.Allocate();
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
		GameObject* ptr = AllocateGameObject();
		auto obj = CreateAt<GameObject>(ptr, *this, std::string{ name });
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

	SH_GAME_API auto World::GetGameObjects() const -> const core::SHashSet<GameObject*>&
	{
		return objs;
	}

	SH_GAME_API auto World::GetGameObjectPool() -> core::memory::MemoryPool<GameObject>&
	{
		return objPool;
	}

	SH_GAME_API void World::PushDeAllocatedGameObject(GameObject* ptr)
	{
		deallocatedObjs.push(ptr);
	}

	SH_GAME_API auto World::DuplicateGameObject(const GameObject& obj) -> GameObject&
	{
		GameObject* ptr = AllocateGameObject();
		auto duplicatedObj = CreateAt<GameObject>(ptr, obj);

		objs.insert(duplicatedObj);

		gc->SetRootSet(duplicatedObj);
		onGameObjectAdded.Notify(duplicatedObj);

		return *duplicatedObj;
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

		imgui->Begin();
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
		imgui->End();
	}

	SH_GAME_API void World::BeforeSync()
	{
		while (!beforeSyncTasks.empty())
		{
			beforeSyncTasks.front()();
			beforeSyncTasks.pop();
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
	SH_GAME_API auto World::GetCameras() const -> const std::unordered_set<Camera*>&
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

	SH_GAME_API void World::AddBeforeSyncTask(const std::function<void()>& func)
	{
		beforeSyncTasks.push(func);
	}

	SH_GAME_API auto World::Serialize() const -> core::Json
	{
		core::Json mainJson = Super::Serialize();

		core::Json objsJson = core::Json::array();
		for (auto obj : objs)
		{
			if (obj->bNotSave)
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
			core::UUID objuuid{ objJson["uuid"].get<std::string>() };

			auto sobj = objManager->GetSObject(objuuid);
			GameObject* gameObj = nullptr;
			if (core::IsValid(sobj)) // 이미 월드 상에 존재 할 경우
			{
				gameObj = static_cast<GameObject*>(sobj);
				// 모든 컴포넌트 제거 (Transform 제외)
				for (int i = 1; i < gameObj->GetComponents().size(); ++i)
				{
					Component* component = gameObj->GetComponents()[i];
					if (component == nullptr)
						continue;
					component->SetUUID(core::UUID::Generate());
					component->Destroy();
				}
			}
			else
			{
				gameObj = this->AddGameObject(name);
				gameObj->SetUUID(objuuid);
			}

			for (auto& compJson : objJson["Components"])
			{
				std::string name{ compJson["name"].get<std::string>() };
				std::string type{ compJson["type"].get<std::string>() };
				core::UUID uuid{ compJson["uuid"].get<std::string>() };
				if (type == "Transform") // 트랜스폼은 게임오브젝트 생성 시 이미 만들어져있다.
				{
					gameObj->transform->SetUUID(core::UUID{ uuid });
					continue;
				}

				auto compType = ComponentModule::GetInstance()->GetComponent(name);
				if (compType == nullptr)
				{
					SH_ERROR_FORMAT("Not found component - {}", type);
					continue;
				}
				Component* component = compType->Create(*gameObj);
				component->SetUUID(core::UUID{ uuid });
				gameObj->AddComponent(component);
			}
		}
		// 역 직렬화
		for (auto& objJson : json["objs"])
		{
			GameObject* obj = static_cast<GameObject*>(objManager->GetSObject(core::UUID{ objJson["uuid"].get<std::string>() }));
			obj->Deserialize(objJson);
		}
	}
	SH_GAME_API auto World::GetUiContext() const -> ImGUImpl&
	{
		return *imgui;
	}
	SH_GAME_API void World::Playing()
	{
		if (playing)
			return;
		playing = true;
		Start();
	}
	SH_GAME_API void World::Stop()
	{
		playing = false;
	}
	SH_GAME_API auto World::IsPlaying() const -> bool
	{
		return playing;
	}
}