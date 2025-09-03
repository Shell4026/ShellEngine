#pragma once
#include "Export.h"

#include <glm/vec3.hpp>
namespace sh::phys
{
	struct HitPoint
	{
		glm::vec3 hitPoint;
		glm::vec3 hitNormal;
	};
}//namespace