#pragma once
#include "Game/Export.h"
#include "Game/Component/Render/LightBase.h"

namespace sh::game
{
	class PointLight : public LightBase
	{
		COMPONENT(PointLight)
	public:
		SH_GAME_API PointLight(GameObject& owner);
		SH_GAME_API ~PointLight();

		SH_GAME_API void BeginUpdate() override;

		SH_GAME_API auto Intersect(const render::AABB& aabb) const -> bool override;

		SH_GAME_API void SetRadius(float radius);

		/// @brief 라이트의 광원 공간 변환 행렬 (proj * view)을 반환한다.
		SH_GAME_API auto GetLightSpaceMatrix() const -> glm::mat4 override;

		SH_GAME_API auto GetShadowViewMatrix() const -> glm::mat4 override;
		SH_GAME_API auto GetShadowProjMatrix() const -> glm::mat4 override;
		SH_GAME_API auto GetShadowPos() const -> glm::vec3 override;
		SH_GAME_API auto GetShadowLookAt() const -> glm::vec3 override;

		SH_GAME_API auto GetPos() const -> const Vec3& override;

		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& prop) override;

		void SetIntensity(float intensity) override { this->intensity = intensity; }
		auto GetIntensity() const -> float override { return intensity; }
		auto GetLightType() const -> ILight::Type override { return ILight::Type::Point; }
		auto GetRadius() const -> float { return range; }
	private:
		PROPERTY(range)
		float range = 5.f;
		PROPERTY(intensity)
		float intensity = 1.f;

		Vec3 lastPos{ 0.f, 0.f, 0.f };
		bool bUpdateOctree = false;
	};
}//namespace