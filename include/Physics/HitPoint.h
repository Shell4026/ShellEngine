#pragma once
#include "Export.h"

#include <glm/vec3.hpp>
namespace sh::phys
{
	struct HitPoint
	{
		float fraction; // 레이에서 이 점이 위치하는 비율
		glm::vec3 hitPoint;
		glm::vec3 hitNormal;
		void* rigidBodyHandle;
	};
}//namespace