#pragma once

#include "Export.h"

#include "Core/Reflection.hpp"

#include "Game/Component/Component.h"

using namespace sh::game;

class ComponentTest : public Component
{
	SCOMPONENT(ComponentTest)
private:
	PROPERTY(height)
	float height;
public:
	SH_USER_API ComponentTest();
	SH_USER_API ~ComponentTest();

	SH_USER_API void OnEnable() override;
	SH_USER_API void Update() override;
};