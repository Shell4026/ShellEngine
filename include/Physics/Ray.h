#pragma once
#include "Export.h"

#include "Game/Vector.h"

namespace sh::phys
{
	class Ray
	{
	public:
		SH_PHYS_API Ray(const game::Vec3& origin, const game::Vec3& dir, float distance = 1000.0f);
	public:
		const game::Vec3 origin;
		const game::Vec3 direction;
		const float distance;
	};
}//namespace