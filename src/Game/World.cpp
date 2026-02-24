#include "World.h"
#include "GameObject.h"
#include "ImGUImpl.h"
#include "WorldEvents.hpp"
#include "AssetLoaderFactory.h"
#include "GameRenderer.h"
#include "Component/Phys/RigidBody.h"
#include "Component/Phys/Collider.h"
#include "Component/Render/Camera.h"

#include "Core/GarbageCollection.h"
#include "Core/Util.h"
#include "Core/SObjectManager.h"
#include "Core/Asset.h"

#include <utility>
#include <cstdint>

namespace sh::game
{
	SH_GAME_API World::World(sh::render::Renderer& renderer, ImGUImpl& guiContext) :
		renderer(renderer), componentModule(*game::ComponentModule::GetInstance()), imgui(&guiContext),
		
		mainCamera(nullptr),
		lightOctree(render::AABB{-1000, -1000, -1000, 1000, 1000, 1000})
	{
		SetName("World");
		gc = core::GarbageCollection::GetInstance();

		physEventSubscriber.SetCallback(
			[](const phys::PhysicsEvent& evt)
			{
				const auto callCollisionFn =
					[](const phys::PhysicsEvent& evt, RigidBody* rb1Ptr, Collider* collider2Ptr)
					{
						if (core::IsValid(rb1Ptr))
						{
							if (evt.type == phys::PhysicsEvent::Type::CollisionEnter ||
								evt.type == phys::PhysicsEvent::Type::CollisionStay ||
								evt.type == phys::PhysicsEvent::Type::CollisionExit)
							{
								if (evt.contactCount < Collision::ARRAY_SIZE)
								{
									std::array<phys::ContactPoint, Collision::ARRAY_SIZE> contactPoint;
									for (uint32_t i = 0; i < evt.contactCount; ++i)
										evt.getContactPointFn(contactPoint[i], i);

									Collision collision{ contactPoint };
									collision.collider = collider2Ptr;
									collision.contactCount = evt.contactCount;
									if (evt.type == phys::PhysicsEvent::Type::CollisionEnter)
										rb1Ptr->gameObject.OnCollisionEnter(std::move(collision));
									else if (evt.type == phys::PhysicsEvent::Type::CollisionStay)
										rb1Ptr->gameObject.OnCollisionStay(std::move(collision));
									else if (evt.type == phys::PhysicsEvent::Type::CollisionExit)
										rb1Ptr->gameObject.OnCollisionExit(std::move(collision));
								}
								else
								{
									std::vector<phys::ContactPoint> contactPoint(evt.contactCount);
									for (uint32_t i = 0; i < evt.contactCount; ++i)
										evt.getContactPointFn(contactPoint[i], i);

									Collision collision{ std::move(contactPoint) };
									collision.collider = collider2Ptr;
									collision.contactCount = evt.contactCount;
									if (evt.type == phys::PhysicsEvent::Type::CollisionEnter)
										rb1Ptr->gameObject.OnCollisionEnter(std::move(collision));
									if (evt.type == phys::PhysicsEvent::Type::CollisionStay)
										rb1Ptr->gameObject.OnCollisionStay(std::move(collision));
									else if (evt.type == phys::PhysicsEvent::Type::CollisionExit)
										rb1Ptr->gameObject.OnCollisionExit(std::move(collision));
								}
							}
							else
							{
								if (!core::IsValid(collider2Ptr))
									return;
								if (evt.type == phys::PhysicsEvent::Type::TriggerEnter)
									rb1Ptr->gameObject.OnTriggerEnter(*collider2Ptr);
								else if (evt.type == phys::PhysicsEvent::Type::TriggerExit)
									rb1Ptr->gameObject.OnTriggerExit(*collider2Ptr);
							}
						}
					};
				RigidBody* const rb1Ptr = RigidBody::GetRigidBodyUsingHandle(evt.rigidBody1Handle);
				RigidBody* const rb2Ptr = RigidBody::GetRigidBodyUsingHandle(evt.rigidBody2Handle);
				Collider* const collider1Ptr = Collider::GetColliderUsingHandle(evt.collider1Handle);
				Collider* const collider2Ptr = Collider::GetColliderUsingHandle(evt.collider2Handle);

				callCollisionFn(evt, rb1Ptr, collider2Ptr);
				callCollisionFn(evt, rb2Ptr, collider1Ptr);
			}
		);
		physWorld.bus.Subscribe(physEventSubscriber);
	}
	SH_GAME_API World::~World()
	{
		SH_INFO_FORMAT("~World {}", GetUUID().ToString());
		Clear();
		physWorld.Clean();
	}

	SH_GAME_API void World::OnDestroy()
	{
		Clear();
		Super::OnDestroy();
	}

	SH_GAME_API void World::Clear()
	{
		CleanObjs();

		while (!deallocatedObjs.empty())
		{
			objPool.DeAllocate(deallocatedObjs.front());
			deallocatedObjs.pop();
		}

		bStartLoop = false;
		bPlaying = false;
		bLoaded = false;
		bOnStart = false;
	}
	SH_GAME_API void World::SetupRenderer()
	{
		uiCamera.SetPriority(1000);
		if (customRenderer == nullptr)
		{
			renderer.AddCamera(uiCamera);
			customRenderer = std::make_unique<GameRenderer>(*renderer.GetContext(), GetUiContext());
			renderer.SetScriptableRenderer(*customRenderer);
		}
		
		static_cast<GameRenderer*>(customRenderer.get())->SetUICamera(uiCamera);
	}
	SH_GAME_API void World::InitResource()
	{
		SH_INFO("Init resource");
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
		for (auto& obj : addedObjs)
		{
			if (obj.IsValid())
				obj->Destroy();
		}
		objIdx.clear();
		objs.clear();
		cameras.clear();
		lightOctree.Clear();

		mainCamera = nullptr;
	}

	SH_GAME_API auto World::AddGameObject(std::string_view name) -> GameObject*
	{
		GameObject* ptr = AllocateGameObject();
		if (ptr == nullptr)
			return nullptr;

		GameObject* obj = CreateAt<GameObject>(ptr, *this, std::string{ name }, GameObject::CreateKey{});
		addedObjs.push_back(obj);
		gc->SetRootSet(obj);

		eventBus.Publish(events::GameObjectEvent{ *obj, events::GameObjectEvent::Type::Added });

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
		if (obj.IsPendingKill())
		{
			auto it = objIdx.find(&obj);
			if (it == objIdx.end())
				return;

			const std::size_t idx = it->second;
			const std::size_t last = objs.size() - 1;
			objIdx.erase(it);

			if (idx != last)
			{
				GameObject* moved = objs[last];
				objs[idx] = moved;
				objIdx[moved] = idx;
			}
			objs.pop_back();
			return;
		}
		obj.Destroy(); // 이 함수에서 월드의 DestroyGameObject()함수가 호출됨
	}

	SH_GAME_API auto World::GetGameObject(std::string_view name) const -> GameObject*
	{
		if (!addedObjs.empty())
		{
			for (auto& obj : addedObjs)
			{
				if (!obj.IsValid())
					continue;
				if (obj->GetName() == name)
					return obj.Get();
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

	SH_GAME_API auto World::GetGameObjectPool() -> core::memory::MemoryPool<GameObject>&
	{
		return objPool;
	}

	SH_GAME_API void World::PushDeAllocatedGameObject(GameObject* ptr)
	{
		deallocatedObjs.push(ptr);
	}

	SH_GAME_API void World::Start()
	{
		bOnStart = true;
	}
	SH_GAME_API void World::Update(float deltaTime)
	{
		dt = deltaTime;
		if (!bStartLoop)
			bStartLoop = true;

		if (bWaitPlaying)
		{
			bPlaying = true;
			bWaitPlaying = false;
			eventBus.Publish(events::WorldEvent{ events::WorldEvent::Type::Play });
		}

		for (auto& addedObj : addedObjs)
		{
			if (addedObj.IsValid())
			{
				objIdx[addedObj.Get()] = objs.size();
				objs.push_back(addedObj.Get());
			}
		}
		addedObjs.clear();

		for (auto& obj : objs)
		{
			if (!core::IsValid(obj))
				continue;
			obj->Awake();
		}
		for (auto& obj : objs)
		{
			if (!core::IsValid(obj))
				continue;
			if (!obj->IsActive())
				continue;
			obj->Start();
		}
		for (auto& obj : objs)
		{
			if (!sh::core::IsValid(obj))
				continue;
			if (!obj->IsActive())
				continue;
			obj->BeginUpdate();
		}
		dtAccumulator += dt;
		while (dtAccumulator >= FIXED_TIME)
		{
			if (bPlaying)
			{
				physWorld.Update(FIXED_TIME);
			}
			for (auto& obj : objs)
			{
				if (!sh::core::IsValid(obj))
					continue;
				if (!obj->IsActive())
					continue;
				obj->FixedUpdate();
			}
			dtAccumulator -= FIXED_TIME;
		}
		for (auto& obj : objs)
		{
			if (!sh::core::IsValid(obj))
				continue;
			if (!obj->IsActive())
				continue;
			obj->ProcessCollisionFunctions();
		}
		for (auto& obj : objs)
		{
			if (!sh::core::IsValid(obj))
				continue;
			if (!obj->IsActive())
				continue;
			obj->Update();
		}
		for (auto& obj : objs)
		{
			if (!sh::core::IsValid(obj))
				continue;
			if (!obj->IsActive())
				continue;
			obj->LateUpdate();
		}
	}

	SH_GAME_API void World::BeforeSync()
	{
		while (!beforeSyncTasks.empty())
		{
			beforeSyncTasks.front()();
			beforeSyncTasks.pop();
		}
	}

	SH_GAME_API void World::AfterSync()
	{
		while (!afterSyncTasks.empty())
		{
			afterSyncTasks.front()();
			afterSyncTasks.pop();
		}
	}

	SH_GAME_API void World::RegisterCamera(Camera* cam)
	{
		if (!core::IsValid(cam))
			return;
		for (int i = 0; i < cameras.size(); ++i)
		{
			if (cameras[i] == cam)
				return;
		}
		cameras.push_back(cam);
		renderer.AddCamera(cam->GetNative());

		eventBus.Publish(events::CameraEvent{ *cam, events::CameraEvent::Type::Added });
	}
	SH_GAME_API void World::UnRegisterCamera(Camera* cam)
	{
		if (cam == nullptr)
			return;
		cameras.erase(std::remove(cameras.begin(), cameras.end(), cam), cameras.end()); // O(n)
		renderer.RemoveCamera(cam->GetNative());

		eventBus.Publish(events::CameraEvent{ *cam, events::CameraEvent::Type::Removed });
	}
	SH_GAME_API void World::SetMainCamera(Camera* cam)
	{
		if (!core::IsValid(cam))
			return;
		mainCamera = cam;
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

	SH_GAME_API void World::AddAfterSyncTask(const std::function<void()>& func)
	{
		afterSyncTasks.push(func);
	}

	SH_GAME_API auto World::Serialize() const -> core::Json
	{
		core::Json mainJson = Super::Serialize();

		core::Json objsJson = core::Json::array();
		for (auto obj : objs)
		{
			if (obj->bNotSave || !core::IsValid(obj))
				continue;
			objsJson.push_back(obj->Serialize());
		}
		mainJson["objs"] = std::move(objsJson);
		return mainJson;
	}
	SH_GAME_API void World::Deserialize(const core::Json& json)
	{
		bLoaded = true;

		Super::Deserialize(json);

		if (!json.contains("objs"))
		{
			bLoaded = false;
			return;
		}

		bool hasCustomRenderer = customRenderer != nullptr;
		CleanObjs();
		if (hasCustomRenderer)
			SetupRenderer();
		InitResource();

		core::SObjectManager* objManager = core::SObjectManager::GetInstance();
		// 유효한 참조를 위해 두번 로드하는 과정을 거친다.
		// 생성만 하는 과정
		for (const auto& objJson : json["objs"])
		{
			const std::string& name = objJson["name"].get_ref<const std::string&>();
			core::UUID objuuid{ objJson["uuid"].get_ref<const std::string&>() };

			auto sobj = objManager->GetSObject(objuuid);
			GameObject* gameObj = nullptr;
			if (core::IsValid(sobj)) // 이미 월드 상에 존재 할 경우
			{
				gameObj = static_cast<GameObject*>(sobj);
				// 모든 컴포넌트 제거
				for (int i = 0; i < gameObj->GetComponents().size(); ++i)
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
				if (sobj != nullptr) // pending kill
					sobj->SetUUID(core::UUID::Generate());
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
					if (gameObj->transform->GetUUID() != uuid)
					{
						// 실패 했다면 이미 해당 트랜스폼이 존재하는 상태
						if (!gameObj->transform->SetUUID(uuid))
						{
							auto obj = core::SObjectManager::GetInstance()->GetSObject(uuid);
							// 보류 상태라면 UUID 변경
							if (obj != nullptr && obj->IsPendingKill())
							{
								obj->SetUUID(core::UUID::Generate());
								gameObj->transform->SetUUID(uuid);
							}
						}
					}
					continue;
				}

				auto compType = ComponentModule::GetInstance()->GetComponent(name);
				if (compType == nullptr)
				{
					SH_ERROR_FORMAT("Not found component - {}", type);
					continue;
				}
				Component* component = compType->Create(*gameObj);
				// 실패 했다면 이미 해당 컴포넌트가 존재하는 상태 (PendingKill상태 일 수도 있음)
				if (!component->SetUUID(core::UUID{ uuid }))
				{
					auto obj = core::SObjectManager::GetInstance()->GetSObject(uuid);
					// 보류 상태라면 UUID 변경
					if (obj != nullptr && obj->IsPendingKill())
					{
						obj->SetUUID(core::UUID::Generate());
						component->SetUUID(core::UUID{ uuid });
					}
				}
				gameObj->AddComponent(component);
			}
		}
		// 역 직렬화
		for (const auto& objJson : json["objs"])
		{
			GameObject* obj = static_cast<GameObject*>(objManager->GetSObject(core::UUID{ objJson["uuid"].get<std::string>() }));
			obj->Deserialize(objJson);
		}
		for (auto obj : objs)
		{
			if (!obj->activeSelf)
				obj->PropagateEnable();
		}
		for (auto& obj : addedObjs)
		{
			if (!obj->activeSelf)
				obj->PropagateEnable();
		}
	}
	SH_GAME_API void World::SaveWorldPoint(const core::Json& json, std::string_view name)
	{
		savePoints[std::string{ name }] = json;
	}
	SH_GAME_API void World::SaveWorldPoint(core::Json&& json, std::string_view name)
	{
		savePoints[std::string{ name }] = std::move(json);
	}
	SH_GAME_API void World::LoadWorldPoint()
	{
		LoadWorldPoint("default");
	}
	SH_GAME_API void World::LoadWorldPoint(const std::string& name)
	{
		auto it = savePoints.find(name);
		if (it == savePoints.end())
			return;

		AddAfterSyncTask(
			[this, savePoint = it->second]()
			{
				Deserialize(savePoint);
			}
		);
	}
	SH_GAME_API auto World::GetWorldPoint() const -> const core::Json*
	{
		return GetWorldPoint("default");
	}
	SH_GAME_API auto World::GetWorldPoint(const std::string& name) const -> const core::Json*
	{
		auto it = savePoints.find(name);
		if (it == savePoints.end())
			return nullptr;
		return &it->second;
	}
	SH_GAME_API auto World::GetWorldPoint() -> core::Json*
	{
		return GetWorldPoint("default");
	}
	SH_GAME_API auto World::GetWorldPoint(const std::string& name) -> core::Json*
	{
		auto it = savePoints.find(name);
		if (it == savePoints.end())
			return nullptr;
		return &it->second;
	}
	SH_GAME_API void World::ClearWorldPoints()
	{
		savePoints.clear();
	}
	SH_GAME_API void World::ClearWorldPoint(const std::string& name)
	{
		savePoints.erase(name);
	}
	SH_GAME_API auto World::GetUiContext() const -> ImGUImpl&
	{
		return *imgui;
	}
	SH_GAME_API void World::PublishEvent(const core::IEvent& event)
	{
		eventBus.Publish(event);
	}
	SH_GAME_API void World::SubscribeEvent(core::ISubscriber& subscriber)
	{
		eventBus.Subscribe(subscriber);
	}
	SH_GAME_API void World::Play()
	{
		if (bPlaying || bWaitPlaying)
			return;

		bWaitPlaying = true;
	}
	SH_GAME_API void World::Stop()
	{
		if (!bPlaying)
			return;

		bPlaying = false;
		bOnStart = false;
		eventBus.Publish(events::WorldEvent{ events::WorldEvent::Type::Stop });
	}
	SH_GAME_API auto World::IsPlaying() const -> bool
	{
		return bPlaying;
	}
	SH_GAME_API auto World::IsStart() const -> bool
	{
		return bOnStart;
	}
	SH_GAME_API auto World::IsLoaded() const -> bool
	{
		return bLoaded;
	}
	SH_GAME_API void World::ReallocateUUIDS()
	{
		auto changeObjUUIDfn =
			[](GameObject* obj)
			{
				obj->SetUUID(core::UUID::Generate());
				for (auto component : obj->GetComponents())
				{
					if (component != nullptr)
						component->SetUUID(core::UUID::Generate());
				}
			};
		for (auto obj : objs)
		{
			changeObjUUIDfn(obj);
		}
		for (auto& obj : addedObjs)
		{
			if (obj.IsValid())
				changeObjUUIDfn(obj.Get());
		}
		SetUUID(core::UUID::Generate());
	}
}//namespace