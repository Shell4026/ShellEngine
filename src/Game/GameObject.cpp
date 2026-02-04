#include "GameObject.h"
#include "World.h"
#include "Prefab.h"
#include "ComponentModule.h"
#include "Component/Component.h"
#include "Component/Phys/Collider.h"

#include "Core/SObjectManager.h"

namespace sh::game
{
	SH_GAME_API GameObject::GameObject(World& world, const std::string& name, CreateKey key) :
		world(world),
		bEnable(true), activeSelf(bEnable), transform(reinterpret_cast<Transform*>(transformBuffer.data()))
	{
		core::GarbageCollection::GetInstance()->SetRootSet(transform);
		core::SObject::CreateAt<Transform>(transformBuffer.data(), *this);
		SetName(name);
	}

	SH_GAME_API GameObject::~GameObject()
	{
		if (bPlacementNew)
			world.PushDeAllocatedGameObject(this);
		SH_INFO_FORMAT("~GameObject: {}", GetName().ToString());
	}

	SH_GAME_API auto GameObject::operator=(const GameObject& other) -> GameObject&
	{
		*transform = *other.transform;
		for (int i = 0; i < other.components.size(); ++i)
		{
			Component* component = other.components[i];
			if (!core::IsValid(component))
				continue;

			IComponentType* componentType = ComponentModule::GetInstance()->GetComponent(component->GetName().ToString());
			if (componentType == nullptr)
			{
				SH_ERROR_FORMAT("Not found component: {}", component->GetName().ToString());
				continue;
			}
			Component* copyComponent = componentType->Clone(*this, *component);
			if (copyComponent != nullptr)
				AddComponent(copyComponent);
		}

		return *this;
	}

	SH_GAME_API void GameObject::Awake()
	{
		if (!transform->IsInit())
			transform->Awake();
		for (auto& component : components)
		{
			if (core::IsValid(component) && !component->IsInit())
			{
				if (world.IsPlaying() || component->canPlayInEditor)
				{
					component->Awake();
					component->bInit = true;
				}
			}
		}
	}
	SH_GAME_API void GameObject::Start()
	{
		if (!transform->IsStart())
			transform->Start();
		for (auto& component : components)
		{
			if (core::IsValid(component) && component->IsActive() && !component->IsStart())
				if (world.IsPlaying() || component->canPlayInEditor)
				{
					component->Start();
					component->bStart = true;
				}
		}
	}

	SH_GAME_API void GameObject::OnEnable()
	{
		for (auto& component : components)
		{
			if(core::IsValid(component) && component->IsActive())
				if (world.IsPlaying() || component->canPlayInEditor)
					component->OnEnable();
		}
	}
	SH_GAME_API void GameObject::OnDisable()
	{
		for (auto& component : components)
		{
			if (core::IsValid(component) && component->IsActive())
				if (world.IsPlaying() || component->canPlayInEditor)
					component->OnDisable();
		}
	}
	SH_GAME_API void GameObject::Destroy()
	{
		Super::Destroy();
		world.DestroyGameObject(*this);
	}
	SH_GAME_API void GameObject::OnDestroy()
	{
		for (auto component : components)
		{
			if (core::IsValid(component))
				component->Destroy();
		}
		transform->Destroy();
		Super::OnDestroy();
	}

	SH_GAME_API void GameObject::BeginUpdate()
	{
		transform->BeginUpdate();
		for (auto& component : components)
		{
			if (core::IsValid(component) && component->IsActive())
				if (world.IsPlaying() || component->canPlayInEditor)
					component->BeginUpdate();
		}
	}
	SH_GAME_API void GameObject::FixedUpdate()
	{
		for (auto& component : components)
		{
			if (core::IsValid(component) && component->IsActive())
				if (world.IsPlaying() || component->canPlayInEditor)
					component->FixedUpdate();
		}
	}
	SH_GAME_API void GameObject::Update()
	{
		for (auto& component : components)
		{
			if (core::IsValid(component) && component->IsActive())
				if (world.IsPlaying() || component->canPlayInEditor)
					component->Update();
		}
	}
	SH_GAME_API void GameObject::LateUpdate()
	{
		for (auto& component : components)
		{
			if (core::IsValid(component) && component->IsActive())
				if (world.IsPlaying() || component->canPlayInEditor)
					component->LateUpdate();
		}
		if (bRequestSortComponent)
		{
			SortComponents();
			bRequestSortComponent = false;
		}
	}

	SH_GAME_API void GameObject::ProcessCollisionFunctions()
	{
		for (auto collider : enterColliders)
		{
			bool bTrigger = collider->IsTrigger();
			if (!core::IsValid(collider))
				continue;

			for (auto& component : components)
			{
				if (core::IsValid(component) && component->IsActive())
				{
					if (world.IsPlaying() || component->canPlayInEditor)
					{
						if (!bTrigger)
							component->OnCollisionEnter(*collider);
						else
							component->OnTriggerEnter(*collider);
					}
				}
			}
		}
		for (auto collider : stayColliders)
		{
			bool bTrigger = collider->IsTrigger();
			if (!core::IsValid(collider))
				continue;

			for (auto& component : components)
			{
				if (core::IsValid(component) && component->IsActive())
				{
					if (world.IsPlaying() || component->canPlayInEditor)
					{
						if (!bTrigger)
							component->OnCollisionStay(*collider);
						else
							component->OnTriggerStay(*collider);
					}
				}
			}
		}
		for (auto collider : exitColliders)
		{
			bool bTrigger = collider->IsTrigger();
			if (!core::IsValid(collider))
				continue;

			stayColliders.erase(collider);

			for (auto& component : components)
			{
				if (core::IsValid(component) && component->IsActive())
				{
					if (world.IsPlaying() || component->canPlayInEditor)
					{
						if (!bTrigger)
							component->OnCollisionExit(*collider);
						else
							component->OnTriggerExit(*collider);
					}
				}
			}
		}
		enterColliders.clear();
		exitColliders.clear();
	}

	SH_GAME_API void GameObject::OnCollisionEnter(Collider& collider)
	{
		if (stayColliders.find(&collider) == stayColliders.end())
		{
			enterColliders.insert(&collider);
			stayColliders.insert(&collider);
		}
	}

	SH_GAME_API void GameObject::OnCollisionExit(Collider& collider)
	{
		exitColliders.insert(&collider);
	}

	SH_GAME_API auto GameObject::Serialize() const -> core::Json
	{
		core::Json mainJson = Super::Serialize();

		core::Json componentJsons = core::Json::array();
		componentJsons.push_back(transform->Serialize());
		for (auto component : components)
		{
			if (!core::IsValid(component))
				continue;
			core::Json componentJson = component->Serialize();
			if (!componentJson.empty())
				componentJsons.push_back(std::move(componentJson));
		}
		mainJson["Components"] = std::move(componentJsons);

		return mainJson;
	}
	SH_GAME_API void GameObject::Deserialize(const core::Json& json)
	{
		Super::Deserialize(json);
		static core::SObjectManager& objManager = *core::SObjectManager::GetInstance();
		for (auto& compJson : json["Components"])
		{
			const std::string& uuid = compJson["uuid"].get_ref<const std::string&>();
			Component* const comp = static_cast<Component*>(objManager.GetSObject(core::UUID{ uuid }));
			if (core::IsValid(comp))
				comp->Deserialize(compJson);
			else
			{
				const std::string& compName{ compJson["name"].get_ref<const std::string&>() };
				auto compType = ComponentModule::GetInstance()->GetComponent(compName);
				if (compType == nullptr)
				{
					SH_ERROR_FORMAT("Not found component - {}", compName);
					continue;
				}
				Component* const component = compType->Create(*this);
				component->SetUUID(core::UUID{ uuid });
				AddComponent(component);
			}
		}
		SortComponents();
	}
	SH_GAME_API void GameObject::OnPropertyChanged(const core::reflection::Property& prop)
	{
		Super::OnPropertyChanged(prop);
		if (prop.GetName() == core::Util::ConstexprHash("bEnable"))
		{
			SetActive(bEnable);
		}
	}

	SH_GAME_API void GameObject::SetActive(bool b)
	{
		const bool bWasActiveInHierarchy = IsActive();

		if (bEnable == b)
			return;

		bEnable = b;
		const bool bIsActiveInHierarchy = IsActive();

		if (bWasActiveInHierarchy == bIsActiveInHierarchy)
			return;

		if (bIsActiveInHierarchy)
			OnEnable();
		else
			OnDisable();

		PropagateEnable();
	}

	SH_GAME_API void GameObject::PropagateEnable()
	{
		std::queue<std::pair<Transform*, bool>> bfs;
		// 현재 계층 활성 상태를 넘겨줌
		for (auto child : transform->GetChildren())
			bfs.push({ child, IsActive() });

		while (!bfs.empty())
		{
			auto [transform, bParentActiveInHierarchy] = bfs.front();
			bfs.pop();

			GameObject& childObj = transform->gameObject;

			bool bWasChildActiveInHierarchy = childObj.IsActive();
			childObj.bParentEnable = bParentActiveInHierarchy;
			bool bNowChildActiveInHierarchy = childObj.IsActive();

			if (!bWasChildActiveInHierarchy && bNowChildActiveInHierarchy)
				childObj.OnEnable();
			else if (bWasChildActiveInHierarchy && !bNowChildActiveInHierarchy)
				childObj.OnDisable();

			for (auto grandChild : transform->GetChildren())
				bfs.push({ grandChild, bNowChildActiveInHierarchy });
		}
	}

	SH_GAME_API void GameObject::AddComponent(Component* component)
	{
		if (!core::IsValid(component) || &component->gameObject != this)
			return;

		components.push_back(std::move(component));
		components.back()->SetActive(true);

		world.PublishEvent(events::ComponentEvent{ *component, events::ComponentEvent::Type::Added });
	}

	SH_GAME_API void GameObject::RequestSortComponents()
	{
		bRequestSortComponent = true;
	}

	SH_GAME_API auto GameObject::Clone() const -> GameObject&
	{
		Prefab* prefab = Prefab::CreatePrefab(*this);
		auto objPtr = prefab->AddToWorld(world);

		return *objPtr;
	}

	void GameObject::SortComponents()
	{
		// nullptr모두 제거
		components.erase(std::remove_if(components.begin(), components.end(), 
			[](const Component* component)
			{
				return !core::IsValid(component);
			}
		), components.end());
		
		std::sort(components.begin(), components.end(),
			[](const Component* left, const Component* right)
			{
				if (!core::IsValid(left))
					return false;
				if (!core::IsValid(right))
					return true;
				return left->GetPriority() > right->GetPriority();
			}
		);
	}
}