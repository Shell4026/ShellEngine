#include "GameObject.h"

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
		SH_INFO_FORMAT("~GameObject: {}", GetName().ToString());
	}

	SH_GAME_API void GameObject::Awake()
	{
		for (auto& Component : components)
		{
			if (Component->active)
				Component->Awake();
		}
		bInit = true;
	}

	SH_GAME_API void GameObject::Start()
	{
		for (auto& Component : components)
		{
			if (Component->active)
				Component->Start();
		}
	}

	SH_GAME_API void GameObject::OnEnable()
	{
		for (auto& Component : components)
		{
			if(Component->active)
				Component->OnEnable();
		}
	}
	SH_GAME_API void GameObject::Destroy()
	{
		for (auto component : components)
		{
			component->Destroy();
		}
		components.clear();
		Super::Destroy();
	}

	SH_GAME_API void GameObject::BeginUpdate()
	{
		for (auto& Component : components)
		{
			if (Component->active)
				Component->BeginUpdate();
		}
	}
	SH_GAME_API void GameObject::FixedUpdate()
	{
		for (auto& Component : components)
		{
			if (Component->active)
				Component->FixedUpdate();
		}
	}
	SH_GAME_API void GameObject::Update()
	{
		for (auto& Component : components)
		{
			if (Component->active)
				Component->Update();
		}
	}
	SH_GAME_API void GameObject::LateUpdate()
	{
		for (auto& Component : components)
		{
			if (Component->active)
				Component->LateUpdate();
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

		onComponentAdd.Notify(components.back());
	}

	SH_GAME_API auto GameObject::Serialize() const -> core::Json
	{
		core::Json mainJson = Super::Serialize();
		core::Json componentJsons = core::Json::array();
		for (auto component : components)
		{
			if (!core::IsValid(component))
				continue;
			componentJsons.push_back(component->Serialize());
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
			Component* comp = static_cast<Component*>(objManager->GetSObject(uuid));
			comp->Deserialize(compJson);
		}
	}
}