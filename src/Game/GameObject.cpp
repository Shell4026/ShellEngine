#include "PCH.h"
#include "GameObject.h"

#include "Component/Component.h"

namespace sh::game
{
	GameObject::GameObject(World& world, const std::string& name) :
		world(world),
		objName(name), name(objName),
		bInit(false), bEnable(true), activeSelf(bEnable)
	{
		transform = AddComponent<Transform>();
	}

	GameObject::GameObject(GameObject&& other) noexcept :
		SObject(std::move(other)),
		world(other.world), transform(other.transform),
		
		objName(std::move(other.objName)), name(objName),
		bInit(other.bInit), bEnable(other.bEnable), activeSelf(bEnable),
		components(std::move(other.components))
	{
		other.transform = nullptr;
	}

	GameObject::~GameObject()
	{
	}

	void GameObject::Awake()
	{
		for (auto& Component : components)
		{
			if (Component->active)
				Component->Awake();
		}
		bInit = true;
	}

	void GameObject::Start()
	{
		for (auto& Component : components)
		{
			if (Component->active)
				Component->Start();
		}
	}

	void GameObject::OnEnable()
	{
		for (auto& Component : components)
		{
			if(Component->active)
				Component->OnEnable();
		}
	}

	void GameObject::BeginUpdate()
	{
		for (auto& Component : components)
		{
			if (Component->active)
				Component->BeginUpdate();
		}
	}

	void GameObject::Update()
	{
		for (auto& Component : components)
		{
			if (Component->active)
				Component->Update();
		}
	}

	void GameObject::LateUpdate()
	{
		for (auto& Component : components)
		{
			if (Component->active)
				Component->LateUpdate();
		}
	}

	void GameObject::Destroy()
	{
		world.DestroyGameObject(name);
	}

	void GameObject::SetActive(bool b)
	{
		bEnable = true;
		if (bEnable)
		{
			if (!bInit)
				Awake();
			OnEnable();
		}
	}

	void GameObject::SetName(const std::string& name)
	{
		objName = world.ChangeGameObjectName(objName, name);
	}

	auto GameObject::GetComponents() const -> const std::vector<Component*>&
	{
		return components;
	}

	void GameObject::AddComponent(Component* component)
	{
		if (!core::IsValid(component) || &component->gameObject != this)
			return;

		components.push_back(std::move(component));
		components.back()->SetActive(true);
	}
}