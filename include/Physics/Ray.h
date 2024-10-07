#pragma once

#include "Export.h"

#include "glm/vec3.hpp"

namespace sh::phys
{
	class Ray
	{
	public:
		const glm::vec3 origin;
		const glm::vec3 direction;
	public:
		SH_PHYS_API Ray(const glm::vec3& origin, const glm::vec3& dir);
	};
}//namespace