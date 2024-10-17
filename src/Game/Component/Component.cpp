#include "PCH.h"
#include "Component/Component.h"

#include "ComponentModule.h"

namespace sh::game
{
	auto Component::GetComponentModule() const -> ComponentModule*
	{
		return ComponentModule::GetInstance();
	}

	SH_GAME_API Component::Component() :
		active(bEnable),
		gameObject(nullptr), bInit(false), bEnable(true)
	{
	}

	SH_GAME_API Component::Component(const Component& other) :
		gameObject(other.gameObject), 
		active(bEnable),
		bInit(false), bEnable(other.bEnable)
	{
	}

	SH_GAME_API Component::Component(Component&& other) noexcept :
		gameObject(other.gameObject),
		active(bEnable),
		bInit(other.bInit), bEnable(other.bEnable)
	{
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
	SH_GAME_API void Component::SetOwner(GameObject& object)
	{
		gameObject = &object;
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
