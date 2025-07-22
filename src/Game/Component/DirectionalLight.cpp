#include "Component/DirectionalLight.h"

#include "GameObject.h"

namespace sh::game
{
	SH_GAME_API DirectionalLight::DirectionalLight(GameObject& owner) :
		Component(owner)
	{
		world.GetLightOctree().Insert(*this);
		canPlayInEditor = true;
	}
	SH_GAME_API DirectionalLight::~DirectionalLight()
	{
	}

	SH_GAME_API void DirectionalLight::OnDestroy()
	{
		world.GetLightOctree().Erase(*this);
	}

	SH_GAME_API bool DirectionalLight::Intersect(const render::AABB& aabb) const
	{
		return true;
	}
	SH_GAME_API void DirectionalLight::SetIntensity(float intensity)
	{
		this->intensity = intensity;
	}
	SH_GAME_API auto DirectionalLight::GetIntensity() const -> float
	{
		return intensity;
	}
	SH_GAME_API void DirectionalLight::SetDirection(const Vec3& dir)
	{
		direction = dir;
	}
	SH_GAME_API auto DirectionalLight::GetDirection() const -> const Vec3&
	{
		return direction;
	}
	SH_GAME_API auto DirectionalLight::GetPos() const -> const Vec3&
	{
		return gameObject.transform->GetWorldPosition();
	}

	SH_GAME_API auto DirectionalLight::GetLightType() const -> ILight::Type
	{
		return ILight::Type::Directional;
	}
}//namespace