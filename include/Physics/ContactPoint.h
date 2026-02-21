#pragma once
#include "glm/vec3.hpp"

namespace sh::phys
{
	struct ContactPoint
	{
		glm::vec3 localPointOnCollider1;
		glm::vec3 localPointOnCollider2;
		glm::vec3 worldNormal;
		float penetrationDepth = 0.f;
	};
}//namespace