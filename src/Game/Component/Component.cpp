#include "PCH.h"
#include "Component/Component.h"

#include "GameObject.h"

namespace sh::game
{
	SH_GAME_API Component::Component(GameObject& object) :
		gameObject(object), world(object.world),
		
		active(bEnable),
		bInit(false), bEnable(true)
	{
	}

	SH_GAME_API Component::Component(const Component& other) :
		SObject(other),
		gameObject(other.gameObject), world(other.world),

		active(bEnable),
		bInit(false), bEnable(other.bEnable)
	{
	}

	SH_GAME_API Component::Component(Component&& other) noexcept :
		SObject(std::move(other)),
		gameObject(other.gameObject), world(other.world),

		active(bEnable),
		bInit(other.bInit), bEnable(other.bEnable)
	{
		other.bEnable = false;
		other.bInit = false;
	}

	SH_GAME_API void Component::SetActive(bool b)
	{
		bEnable = b;
		if (bEnable)
		{
			if (!bInit)
			{
				Awake();
				bInit = true;
			}
			OnEnable();
		}
	}

	SH_GAME_API void Component::Awake()
	{
	}
	SH_GAME_API void Component::Start()
	{

	}
	SH_GAME_API void Component::OnEnable()
	{

	}

	SH_GAME_API void Component::BeginUpdate()
	{
	}
	SH_GAME_API void Component::Update()
	{
	}
	SH_GAME_API void Component::LateUpdate()
	{
	}
	
	SH_GAME_API void Component::OnDestroy()
	{
	}
}
