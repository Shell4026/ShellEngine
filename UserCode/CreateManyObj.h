#pragma once

#include "Export.h"

#include "Game/Component/Component.h"

class CreateManyObj : public sh::game::Component
{
	COMPONENT(CreateManyObj, "user")
private:
	PROPERTY(count)
	int count = 10;
public:
	SH_USER_API CreateManyObj(sh::game::GameObject& owner);
	SH_USER_API ~CreateManyObj() = default;

	SH_USER_API void Update() override;
};