#pragma once

#include "Export.h"

#include "Core/Reflection.hpp"

#include "Game/Component/Component.h"

using namespace sh::game;

class ComponentTest : public Component
{
	COMPONENT(ComponentTest, "user")
private:
	PROPERTY(target)
	std::string target;
public:
	SH_USER_API ComponentTest(GameObject& owner);
	SH_USER_API ~ComponentTest();

	SH_USER_API void OnEnable() override;
	SH_USER_API void Update() override;

	SH_USER_API void OnPropertyChanged(const sh::core::reflection::Property& prop) override;
};