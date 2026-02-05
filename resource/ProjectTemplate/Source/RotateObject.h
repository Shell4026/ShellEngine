#pragma once
#include "Export.h"

#include "Game/Component/Component.h"

namespace sh::game
{
	class RotateObject : public Component
	{
		COMPONENT(RotateObject, "user")
	public:
		SH_USER_API RotateObject(GameObject& owner);
		SH_USER_API ~RotateObject();

		SH_USER_API void OnEnable() override;
		SH_USER_API void Update() override;
	private:
		PROPERTY(speed)
		float speed;
		PROPERTY(xspeed)
		float xspeed = 0;
		PROPERTY(zspeed)
		float zspeed = 0;
	};
}//namespace