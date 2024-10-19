#include "PCH.h"
#include "GameObject.h"

#include "Component/Component.h"

namespace sh::game
{
	SH_GAME_API GameObject::GameObject(World& world, const std::string& name) :
		world(world),
		objName(name), name(objName),
		bInit(false), bEnable(true), activeSelf(bEnable)
	{
		transform = AddComponent<Transform>();
	}

	SH_GAME_API GameObject::GameObject(GameObject&& other) noexcept :
		SObject(std::move(other)),
		world(other.world), transform(other.transform),
		
		objName(std::move(other.objName)), name(objName),
		bInit(other.bInit), bEnable(other.bEnable), activeSelf(bEnable),
		components(std::move(other.components))
	{
		other.transform = nullptr;
	}

	SH_GAME_API GameObject::~GameObject()
	{
		SH_INFO_FORMAT("~GameObject: {}", name);
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
	SH_GAME_API void GameObject::Destroy()
	{
		world.DestroyGameObject(name);
		for (auto component : components)
		{
			component->Destroy();
		}
		SObject::Destroy();
	}
	SH_GAME_API void GameObject::SetActive(bool b)
	{
		bEnable = true;
		if (bEnable)
		{
			if (!bInit)
				Awake();
			OnEnable();
		}
	}

	SH_GAME_API void GameObject::SetName(const std::string& name)
	{
		objName = world.ChangeGameObjectName(objName, name);
	}

	SH_GAME_API auto GameObject::GetComponents() const -> const std::vector<Component*>&
	{
		return components;
	}

	SH_GAME_API void GameObject::AddComponent(Component* component)
	{
		if (!core::IsValid(component) || &component->gameObject != this)
			return;

#if SH_EDITOR
		component->editorName = component->GetType().typeName;
#endif
		components.push_back(std::move(component));
		components.back()->SetActive(true);
	}
}