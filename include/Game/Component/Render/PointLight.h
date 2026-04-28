#pragma once
#include "Game/Export.h"
#include "Game/ILight.h"
#include "Game/Component/Component.h"

namespace sh::game
{
	class PointLight : public Component, public ILight
	{
		COMPONENT(PointLight)
	private:
		PROPERTY(range)
		float range = 5.f;
		PROPERTY(intensity)
		float intensity = 1.f;

		PROPERTY(bCastShadow)
		bool bCastShadow = false;
		PROPERTY(shadowBias)
		float shadowBias = 0.005f;
		PROPERTY(shadowMapResolution)
		uint32_t shadowMapResolution = 512;

		Vec3 lastPos{ 0.f, 0.f, 0.f };
		bool bUpdateOctree = false;
	public:
		SH_GAME_API PointLight(GameObject& owner);
		SH_GAME_API ~PointLight();

		SH_GAME_API void OnDestroy() override;

		SH_GAME_API void BeginUpdate() override;

		SH_GAME_API bool Intersect(const render::AABB& aabb) const override;

		SH_GAME_API void SetRadius(float radius);
		SH_GAME_API void SetIntensity(float intensity);

		SH_GAME_API auto GetRadius() const -> float;
		SH_GAME_API auto GetIntensity() const -> float;
		SH_GAME_API auto GetPos() const -> const Vec3& override;
		SH_GAME_API auto GetLightType() const -> ILight::Type override;

		SH_GAME_API auto IsCastShadow() const -> bool override;
		SH_GAME_API void SetCastShadow(bool b) override;
		SH_GAME_API auto GetShadowBias() const -> float override;
		SH_GAME_API void SetShadowBias(float bias) override;
		SH_GAME_API auto GetShadowMapResolution() const -> uint32_t override;
		SH_GAME_API void SetShadowMapResolution(uint32_t res) override;

		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& prop) override;
	};
}//namespace