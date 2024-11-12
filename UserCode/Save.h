#pragma once
#include "Export.h"
#include "Game/Component/Component.h"

class Save : public sh::game::Component
{
	COMPONENT(Save)
public:
	SH_USER_API Save(sh::game::GameObject& owner);
	SH_USER_API void Update() override;
};