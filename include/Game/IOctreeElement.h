#pragma once

#include "Vector.h"

namespace sh::render
{
	class AABB;
}

namespace sh::game
{
	class IOctreeElement
	{
	public:
		virtual ~IOctreeElement() = default;
		virtual auto GetPos() const -> const Vec3& = 0;
		virtual bool Intersect(const render::AABB& aabb) const = 0;
	};
}//namespace