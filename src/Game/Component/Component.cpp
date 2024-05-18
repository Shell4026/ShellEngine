#include "Component/Component.h"

namespace sh::game
{
	Component::Component(GameObject& owner) :
		gameObject(owner), bInit(false), bEnable(true), active(bEnable)
	{
	}

	Component::Component(const Component& other) :
		gameObject(other.gameObject), 
		active(bEnable),
		bInit(false), bEnable(other.bEnable)
	{
	}

	Component::Component(Component&& other) noexcept :
		gameObject(other.gameObject),
		active(bEnable),
		bInit(other.bInit), bEnable(other.bEnable)
	{
	}

	void Component::SetActive(bool b)
	{
		bEnable = b;
		if (bEnable)
		{
			if (!bInit)
				Awake();
			OnEnable();
		}
	}

	void Component::Awake()
	{
		bInit = true;
	}
	void Component::Start()
	{

	}
	void Component::OnEnable()
	{

	}
	void Component::Update()
	{

	}
	void Component::LateUpdate()
	{

	}
}
