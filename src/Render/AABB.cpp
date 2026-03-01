#include "pch.h"
#include "AABB.h"

namespace sh::render
{
	SH_RENDER_API AABB::AABB() :
		AABB(-0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f)
	{}
	SH_RENDER_API AABB::AABB(const glm::vec3& min, const glm::vec3& max) :
		AABB(min.x, min.y, min.z, max.x, max.y, max.z)
	{}
	SH_RENDER_API AABB::AABB(float x1, float y1, float z1, float x2, float y2, float z2) :
		min(x1, y1, z1), max(x2, y2, z2)
	{
		NormalizeBounds();
		UpdateCenter();
		radius = glm::length(max - center);
	}

	SH_RENDER_API void AABB::Set(const glm::vec3& min, const glm::vec3& max)
	{
		this->min = min;
		this->max = max;
		NormalizeBounds();
		UpdateCenter();
		radius = glm::length(max - center);
	}
	SH_RENDER_API void AABB::Expand(float amount)
	{
		this->min -= amount;
		this->max += amount;
		NormalizeBounds();
		UpdateCenter();
		radius = glm::length(max - center);
	}

	SH_RENDER_API bool AABB::Contains(const glm::vec3& point) const
	{
		if (point.x < min.x || point.x > max.x)
			return false;
		if (point.y < min.y || point.y > max.y)
			return false;
		if (point.z < min.z || point.z > max.z)
			return false;
		return true;
	}
	SH_RENDER_API bool AABB::Intersects(const AABB& other) const
	{
		if (max.x < other.min.x || min.x > other.max.x)
			return false;
		if (max.y < other.min.y || min.y > other.max.y)
			return false;
		if (max.z < other.min.z || min.z > other.max.z)
			return false;
		return true;
	}
	SH_RENDER_API auto AABB::GetWorldAABB(const glm::mat4& modelMatrix) const -> AABB
	{
		std::array<glm::vec3, 8> corners;
		corners[0] = { min.x, min.y, min.z };
		corners[1] = { min.x, min.y, max.z };
		corners[2] = { min.x, max.y, min.z };
		corners[3] = { min.x, max.y, max.z };
		corners[4] = { max.x, min.y, min.z };
		corners[5] = { max.x, min.y, max.z };
		corners[6] = { max.x, max.y, min.z };
		corners[7] = { max.x, max.y, max.z };

		std::array<glm::vec3, 8> transformedCorners;
		for (int i = 0; i < 8; ++i)
			transformedCorners[i] = modelMatrix * glm::vec4(corners[i], 1.0f);

		glm::vec3 newMin = 
		{ 
			std::numeric_limits<float>::max(),
			std::numeric_limits<float>::max(),
			std::numeric_limits<float>::max() 
		};
		glm::vec3 newMax = 
		{ 
			std::numeric_limits<float>::lowest(),
			std::numeric_limits<float>::lowest(),
			std::numeric_limits<float>::lowest() 
		};

		for (const auto& corner : transformedCorners) 
		{
			newMin.x = std::min(newMin.x, corner.x);
			newMin.y = std::min(newMin.y, corner.y);
			newMin.z = std::min(newMin.z, corner.z);

			newMax.x = std::max(newMax.x, corner.x);
			newMax.y = std::max(newMax.y, corner.y);
			newMax.z = std::max(newMax.z, corner.z);
		}

		return AABB{ newMin, newMax };
	}
	SH_RENDER_API auto AABB::Encapsulate(const AABB& a, const AABB& b) -> AABB
	{
		AABB aabb{};
		aabb.min.x = std::min(a.min.x, b.min.x);
		aabb.min.y = std::min(a.min.y, b.min.y);
		aabb.min.z = std::min(a.min.z, b.min.z);

		aabb.max.x = std::max(a.max.x, b.max.x);
		aabb.max.y = std::max(a.max.y, b.max.y);
		aabb.max.z = std::max(a.max.z, b.max.z);

		aabb.center = (aabb.min + aabb.max) / 2.0f;

		return aabb;
	}
	void AABB::NormalizeBounds()
	{
		if (min.x > max.x)
			std::swap(min.x, max.x);
		if (min.y > max.y)
			std::swap(min.y, max.y);
		if (min.z > max.z)
			std::swap(min.z, max.z);
	}
	void AABB::UpdateCenter()
	{
		center = (min + max) / 2.0f;
	}
}//namespace
