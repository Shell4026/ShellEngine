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
		world(other.world), objName(std::move(other.objName)), name(objName),
		bInit(other.bInit), bEnable(other.bEnable), activeSelf(bEnable),
		components(std::move(other.components)), transform(other.transform)
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

	auto GameObject::GetComponents() const -> const std::vector<std::unique_ptr<Component>>&
	{
		return components;
	}

	void GameObject::AddComponent(std::unique_ptr<Component>&& component)
	{
		component->SetOwner(*this);
		gc->SetRootSet(component.get());
		components.push_back(std::move(component));
		components.back()->SetActive(true);
	}
}