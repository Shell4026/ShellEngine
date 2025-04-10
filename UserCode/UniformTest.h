﻿#pragma once

#include "Game/Component/Component.h"

#include "Game/Export.h"

#include "Render/Material.h"

namespace sh::game
{
	class UniformTest : public Component
	{
		COMPONENT(UniformTest, "user")
	private:
		PROPERTY(mat)
		sh::render::Material* mat;

		float t = 0;
	public:
		SH_GAME_API UniformTest(GameObject& owner);

		SH_GAME_API void SetMaterial(sh::render::Material& mat);
		SH_GAME_API void Update() override;
	};
}