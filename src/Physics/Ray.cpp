#include "PCH.h"
#include "Ray.h"

namespace sh::phys
{
	Ray::Ray(const glm::vec3& origin, const glm::vec3& dir) :
		origin(origin), direction(dir)
	{
	}
}//namespace