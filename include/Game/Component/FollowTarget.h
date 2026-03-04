#pragma once
#include "Game/Export.h"
#include "Game/Component/Component.h"

namespace sh::game
{
	class Transform;
	class FollowTarget : public Component
	{
		COMPONENT(FollowTarget)
	public:
		SH_GAME_API FollowTarget(GameObject& owner);

		SH_GAME_API void Update() override;
	private:
		PROPERTY(target, core::PropertyOption::sobjPtr)
		Transform* target = nullptr;
		PROPERTY(speed)
		float speed = 1.f;
	};
}//namespace