#include "GameObject.h"
#include "ComponentModule.h"
#include "Component/Component.h"

#include "Core/SObjectManager.h"

namespace sh::game
{
	SH_GAME_API GameObject::GameObject(World& world, const std::string& name) :
		world(world),
		bInit(false), bEnable(true), activeSelf(bEnable)
	{
		transform = AddComponent<Transform>();
		SetName(name);
	}

	GameObject::GameObject(const GameObject& other) :
		SObject(other),
		world(other.world), activeSelf(bEnable),
		bInit(other.bInit), bEnable(other.bEnable), hideInspector(other.hideInspector), bNotSave(other.bNotSave)
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

	SH_GAME_API GameObject::GameObject(GameObject&& other) noexcept :
		SObject(std::move(other)),
		world(other.world), transform(other.transform),
		
		bInit(other.bInit), bEnable(other.bEnable), activeSelf(bEnable),
		components(std::move(other.components))
	{
		other.transform = nullptr;
	}

	SH_GAME_API GameObject::~GameObject()
	{
		if (bPlacementNew)
			world.PushDeAllocatedGameObject(this);
		SH_INFO_FORMAT("~GameObject: {}", GetName().ToString());
	}

	SH_GAME_API void GameObject::Awake()
	{
		if (bInit)
			return;

		for (auto& component : components)
		{
			if (core::IsValid(component) && component->IsActive() && !component->IsInit())
			{
				if (world.IsPlaying() || component->canPlayInEditor)
					component->Awake();
			}
		}
		bInit = true;
	}

	SH_GAME_API void GameObject::Start()
	{
		for (auto& component : components)
		{
			if (core::IsValid(component) && component->IsActive())
				if (world.IsPlaying() || component->canPlayInEditor)
					component->Start();
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
	SH_GAME_API void GameObject::Destroy()
	{
		for (auto component : components)
		{
			if (core::IsValid(component))
				component->Destroy();
		}
		components.clear();
		Super::Destroy();
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
	}

	SH_GAME_API void GameObject::SetActive(bool b)
	{
		if (!bEnable && b == true)
		{
			if (!bInit)
				Awake();
			OnEnable();
		}
		bEnable = b;
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
	}
}