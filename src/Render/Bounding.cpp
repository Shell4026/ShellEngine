#include "pch.h"
#include "Bounding.h"

namespace sh::render
{
	Bounding::Bounding() :
		Bounding(-0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f)
	{}
	Bounding::Bounding(const glm::vec3& min, const glm::vec3& max) :
		Bounding(min.x, min.y, min.z, max.x, max.y, max.z)
	{}
	Bounding::Bounding(float x1, float y1, float z1, float x2, float y2, float z2) :
		min(x1, y1, z1), max(x2, y2, z2)
	{
		center = (min + max) / 2.0f;
	}

	void Bounding::Set(const glm::vec3& min, const glm::vec3& max)
	{
		this->min = min;
		this->max = max;
		center = (min + max) / 2.0f;
	}
	void Bounding::Expand(float amount)
	{
		this->min -= amount;
		this->max += amount;
		center = (min + max) / 2.0f;
	}

	bool Bounding::Contains(const glm::vec3& point) const
	{
		if (point.x < min.x || point.x > max.x)
			return false;
		if (point.y < min.y || point.y > max.y)
			return false;
		if (point.z < min.z || point.z > max.z)
			return false;
		return true;
	}
	bool Bounding::Intersects(const Bounding& other) const
	{
		if (max.x < other.min.x || min.x > other.max.x)
			return false;
		if (max.y < other.min.y || min.y > other.max.y)
			return false;
		if (max.z < other.min.z || min.z > other.max.z)
			return false;
		return true;
	}

	auto Bounding::GetMin() const -> const glm::vec3&
	{
		return min;
	}
	auto Bounding::GetMax() const -> const glm::vec3&
	{
		return max;
	}
	auto Bounding::GetCenter() const -> const glm::vec3&
	{
		return center;
	}
}//namespace