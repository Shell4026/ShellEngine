#include "PCH.h"
#include "component/PointLight.h"

#include "GameObject.h"

namespace sh::game
{
	SH_GAME_API PointLight::PointLight(GameObject& owner) :
		ILight(owner)
	{
		world.GetLightOctree().Insert(*this);
	}

	SH_GAME_API void PointLight::BeginUpdate()
	{
		if (lastPos != gameObject.transform->position)
			bUpdateOctree = true;
		if (!bUpdateOctree)
			return;

		world.GetLightOctree().Erase(*this);
		world.GetLightOctree().Insert(*this);
	}

	SH_GAME_API bool PointLight::Intersect(const render::AABB& aabb) const
	{
		Vec3 pos = GetPos();
		const glm::vec3& center = aabb.GetCenter();
		const glm::vec3& min = aabb.GetMin();
		const glm::vec3& max = aabb.GetMax();
		float sqDist = 0.0f;

		// x축
		if (pos.x < min.x) 
		{
			float d = pos.x - min.x;
			sqDist += d * d;
		}
		else if (pos.x > max.x) 
		{
			float d = pos.x - max.x;
			sqDist += d * d;
		}

		// y축
		if (pos.y < min.y) {
			float d = pos.y - min.y;
			sqDist += d * d;
		}
		else if (pos.y > max.y) {
			float d = pos.y - max.y;
			sqDist += d * d;
		}

		// z축
		if (pos.z < min.z) {
			float d = pos.z - min.z;
			sqDist += d * d;
		}
		else if (pos.z > max.z) {
			float d = pos.z - max.z;
			sqDist += d * d;
		}

		// 구의 반지름과 비교
		return sqDist <= (range * range);
	}

	SH_GAME_API void PointLight::SetRadius(float radius)
	{
		this->range = radius;
		bUpdateOctree = true;
	}
	SH_GAME_API void PointLight::SetIntensity(float intensity)
	{
		this->intensity = intensity;
	}
	SH_GAME_API auto PointLight::GetRadius() const -> float
	{
		return range;
	}
	SH_GAME_API auto PointLight::GetIntensity() const -> float
	{
		return intensity;
	}
	SH_GAME_API auto PointLight::GetPos() const -> const Vec3&
	{
		return gameObject.transform->position;
	}
}//namespace