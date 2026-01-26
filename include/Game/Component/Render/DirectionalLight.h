#pragma once
#include "Game/Export.h"
#include "ILight.h"
#include "Game/Component/Component.h"

namespace sh::game
{
	class DirectionalLight : public Component, public ILight
	{
		COMPONENT(DirectionalLight)
	private:
		PROPERTY(direction)
		Vec3 direction{ -1.f, -1.f, -1.f };
		PROPERTY(intensity)
		float intensity = 1.f;
	public:
		SH_GAME_API DirectionalLight(GameObject& owner);
		SH_GAME_API ~DirectionalLight();

		SH_GAME_API void OnDestroy() override;

		SH_GAME_API bool Intersect(const render::AABB& aabb) const override;

		SH_GAME_API void SetIntensity(float intensity);
		SH_GAME_API auto GetIntensity() const -> float;

		SH_GAME_API void SetDirection(const Vec3& dir);
		SH_GAME_API auto GetDirection() const -> const Vec3&;

		SH_GAME_API auto GetPos() const -> const Vec3& override;
		SH_GAME_API auto GetLightType() const -> ILight::Type override;
	};
}//namespace