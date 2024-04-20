#pragma once

#include "Game/Export.h"
#include "Component.h"

#include <glm/vec3.hpp>

namespace sh::game
{
	class Transform : public Component
	{
		SCLASS(Transform)
	private:
		glm::vec3 localPosition;
		glm::vec3 position;
	public:
		SH_GAME_API Transform(GameObject& owner);
		SH_GAME_API ~Transform();

		SH_GAME_API void Awake() override;
		SH_GAME_API void Start() override;
	};
}