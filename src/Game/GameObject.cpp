#include "GameObject.h"
#include "GameManager.h"
#include "ComponentModule.h"
#include "Component/Component.h"
#include "Component/Collider.h"

#include "Core/SObjectManager.h"

namespace sh::game
{
	SH_GAME_API GameObject::GameObject(World& world, const std::string& name) :
		world(world),
		bEnable(true), activeSelf(bEnable)
	{
		transform = AddComponent<Transform>();
		SetName(name);
	}

	GameObject::GameObject(const GameObject& other) :
		SObject(other),
		world(other.world), activeSelf(bEnable),
		bEnable(other.bEnable), hideInspector(other.hideInspector), bNotSave(other.bNotSave)
	{
		transform = AddComponent<Transform>();
		*transform = *other.transform;

		for (int i = 1; i < other.components.size(); ++i)
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
			if (copyComponent)
				AddComponent(copyComponent);
		}
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
		for (int i = 1; i < other.components.size(); ++i)
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
	SH_GAME_API void GameObject::OnDestroy()
	{
		for (auto component : components)
		{
			if (core::IsValid(component))
				component->Destroy();
		}
		Super::OnDestroy();
	}

	SH_GAME_API void GameObject::BeginUpdate()
	{
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
			if (!core::IsValid(collider))
				continue;

			for (auto& component : components)
			{
				if (core::IsValid(component) && component->IsActive())
					if (world.IsPlaying() || component->canPlayInEditor)
						component->OnCollisionEnter(*collider);
			}
		}
		for (auto collider : stayColliders)
		{
			if (!core::IsValid(collider))
				continue;

			for (auto& component : components)
			{
				if (core::IsValid(component) && component->IsActive())
					if (world.IsPlaying() || component->canPlayInEditor)
						component->OnCollisionStay(*collider);
			}
		}
		for (auto collider : exitColliders)
		{
			if (!core::IsValid(collider))
				continue;

			stayColliders.erase(collider);

			for (auto& component : components)
			{
				if (core::IsValid(component) && component->IsActive())
					if (world.IsPlaying() || component->canPlayInEditor)
						component->OnCollisionExit(*collider);
			}
		}
		enterColliders.clear();
		exitColliders.clear();
	}

	SH_GAME_API void GameObject::OnCollisionEnter(Collider& collider)
	{
		if (stayColliders.find(&collider) == stayColliders.end())
			enterColliders.insert(&collider);
	}

	SH_GAME_API void GameObject::OnCollisionStay(Collider& collider)
	{
		stayColliders.insert(&collider);
	}

	SH_GAME_API void GameObject::OnCollisionExit(Collider& collider)
	{
		exitColliders.insert(&collider);
	}

	SH_GAME_API void GameObject::SetActive(bool b)
	{
		if (!bEnable && b)
			OnEnable();
		bEnable = b;

		if (bParentEnable)
		{
			std::queue<Transform*> bfs;
			for (auto child : transform->GetChildren())
				bfs.push(child);

			while (!bfs.empty())
			{
				Transform* transform = bfs.front();
				bfs.pop();

				if (!transform->gameObject.bParentEnable && bEnable && activeSelf)
					OnEnable();
				transform->gameObject.bParentEnable = bEnable;

				if (!transform->gameObject.activeSelf)
					continue;

				for (auto child : transform->GetChildren())
					bfs.push(child);
			}
		}
	}

	SH_GAME_API auto GameObject::IsParentActive() const -> bool
	{
		return bParentEnable;
	}

	SH_GAME_API auto GameObject::GetComponents() const -> const std::vector<Component*>&
	{
		return components;
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
		return world.DuplicateGameObject(*this);
	}

	SH_GAME_API auto GameObject::Serialize() const -> core::Json
	{
		core::Json mainJson = Super::Serialize();
		core::Json componentJsons = core::Json::array();
		for (auto component : components)
		{
			if (!core::IsValid(component))
				continue;
			core::Json componentJson = component->Serialize();
			if (!componentJson.empty())
				componentJsons.push_back(std::move(componentJson));
		}
		mainJson["Components"] = componentJsons;
		return mainJson;
	}
	SH_GAME_API void GameObject::Deserialize(const core::Json& json)
	{
		Super::Deserialize(json);
		core::SObjectManager* objManager = core::SObjectManager::GetInstance();
		for (auto& compJson : json["Components"])
		{
			std::string uuid = compJson["uuid"].get<std::string>();
			Component* comp = static_cast<Component*>(objManager->GetSObject(core::UUID{ uuid }));
			if (core::IsValid(comp))
				comp->Deserialize(compJson);
			else
			{
				std::string compName{ compJson["name"].get<std::string>() };
				auto compType = ComponentModule::GetInstance()->GetComponent(compName);
				if (compType == nullptr)
				{
					SH_ERROR_FORMAT("Not found component - {}", compName);
					continue;
				}
				Component* component = compType->Create(*this);
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