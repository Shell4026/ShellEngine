#include "Component/Component.h"

namespace sh::game
{
	Component::Component(GameObject& owner) :
		gameObject(owner), bInit(false), bEnable(true), active(bEnable)
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
