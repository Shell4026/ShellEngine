#pragma once
#include "Game/Export.h"
#include "Game/Component/Render/LightBase.h"

#include "Render/RenderData.h"
#include "Render/ShadowMapManager.h"

#include <glm/mat4x4.hpp>

namespace sh::render
{
	class RenderTexture;
}

namespace sh::game
{
	class DirectionalLight : public LightBase
	{
		COMPONENT(DirectionalLight)
	public:
		SH_GAME_API DirectionalLight(GameObject& owner);
		SH_GAME_API ~DirectionalLight();

		SH_GAME_API auto Intersect(const render::AABB& aabb) const -> bool override;

		SH_GAME_API void SetIntensity(float intensity) override;
		SH_GAME_API void SetDirection(const Vec3& dir);
		/// @brief 라이트의 광원 공간 변환 행렬 (proj * view)을 반환한다.
		SH_GAME_API auto GetLightSpaceMatrix() const -> glm::mat4 override;

		SH_GAME_API auto GetShadowViewMatrix() const -> glm::mat4 override;
		SH_GAME_API auto GetShadowProjMatrix() const -> glm::mat4 override;
		SH_GAME_API auto GetShadowPos() const -> glm::vec3 override;
		SH_GAME_API auto GetShadowLookAt() const -> glm::vec3 override;

		SH_GAME_API auto GetPos() const -> const Vec3& override;

		auto GetLightType() const -> ILight::Type override { return ILight::Type::Directional; }
		auto GetIntensity() const -> float override { return intensity; }
		auto GetDirection() const -> const Vec3& { return direction; }
	private:
		PROPERTY(direction)
		Vec3 direction{ -1.f, -1.f, -1.f };
		PROPERTY(intensity)
		float intensity = 1.f;
	};
}//namespace
