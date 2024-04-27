#include "GameObject.h"

#include "SObject.h"
#include "Component/Component.h"

namespace sh::game
{
	GameObject::GameObject(World& world, const std::string& name) :
		world(world), objName(name), name(objName),
		bInit(false), bEnable(true), activeSelf(bEnable)
	{
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

	auto GameObject::GetComponents() const -> const std::vector<std::unique_ptr<Component>>&
	{
		return components;
	}
}