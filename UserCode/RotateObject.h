#pragma once

#include "Export.h"

#include "Game/Component/Component.h"

class RotateObject : public sh::game::Component
{
	COMPONENT(RotateObject, "user")
private:
	PROPERTY(speed)
	float speed;
public:
	SH_USER_API RotateObject(sh::game::GameObject& owner);
	SH_USER_API ~RotateObject();

	SH_USER_API void OnEnable() override;
	SH_USER_API void Update() override;
};