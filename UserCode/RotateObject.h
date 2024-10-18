#pragma once

#include "Export.h"

#include "Game/Component/Component.h"

using namespace sh::game;

class RotateObject : public Component
{
	COMPONENT(RotateObject, "user")
private:
	PROPERTY(speed)
	float speed;
public:
	SH_USER_API RotateObject(GameObject& owner);
	SH_USER_API ~RotateObject();

	SH_USER_API void OnEnable() override;
	SH_USER_API void Update() override;
};