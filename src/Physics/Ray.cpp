#include "Ray.h"

namespace sh::phys
{
	Ray::Ray(const game::Vec3& origin, const game::Vec3& dir, float distance) :
		origin(origin), direction(dir), distance(distance)
	{
	}
}//namespace