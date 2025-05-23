﻿#pragma once
#include "Export.h"
#include "ILight.h"
#include "Component.h"

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

		Vec3 lastPos{ 0.f, 0.f, 0.f };
		bool bUpdateOctree = false;
	public:
		SH_GAME_API PointLight(GameObject& owner);
		SH_GAME_API ~PointLight();

		SH_GAME_API void Destroy() override;

		SH_GAME_API void BeginUpdate() override;

		SH_GAME_API bool Intersect(const render::AABB& aabb) const override;

		SH_GAME_API void SetRadius(float radius);
		SH_GAME_API void SetIntensity(float intensity);

		SH_GAME_API auto GetRadius() const -> float;
		SH_GAME_API auto GetIntensity() const -> float;
		SH_GAME_API auto GetPos() const -> const Vec3& override;
		SH_GAME_API auto GetLightType() const -> ILight::Type override;
		
		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& prop) override;
	};
}//namespace