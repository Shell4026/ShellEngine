#pragma once

#include "Export.h"

#include "Vector.h"

#include <optional>

namespace sh::game
{
	struct SMath
	{
		SH_GAME_API static auto GetPlaneCollisionPoint(const glm::vec3& linePoint, const glm::vec3& lineDir, const glm::vec3& planePoint, const glm::vec3& planeNormal) -> std::optional<glm::vec3>;
	};
}