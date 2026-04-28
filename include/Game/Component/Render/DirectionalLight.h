#pragma once
#include "Game/Export.h"
#include "Game/ILight.h"
#include "Game/Component/Component.h"

#include "Render/Camera.h"
#include "Render/RenderTexture.h"

#include <glm/mat4x4.hpp>

namespace sh::game
{
	class DirectionalLight : public Component, public ILight
	{
		COMPONENT(DirectionalLight)
	public:
		SH_GAME_API DirectionalLight(GameObject& owner);
		SH_GAME_API ~DirectionalLight();

		SH_GAME_API void Awake() override;
		SH_GAME_API void BeginUpdate() override;
		SH_GAME_API void OnDestroy() override;
		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& prop) override;

		SH_GAME_API bool Intersect(const render::AABB& aabb) const override;

		SH_GAME_API void SetIntensity(float intensity) override;
		SH_GAME_API void SetDirection(const Vec3& dir);
		SH_GAME_API void SetCastShadow(bool b) override;
		SH_GAME_API void SetShadowBias(float bias) override;
		SH_GAME_API void SetShadowMapResolution(uint32_t res) override;

		SH_GAME_API auto GetPos() const -> const Vec3 & override;
		SH_GAME_API auto GetLightType() const -> ILight::Type override;
		SH_GAME_API auto IsCastShadow() const -> bool override { return bCastShadow; }
		SH_GAME_API auto GetShadowBias() const -> float override { return shadowBias; }
		SH_GAME_API auto GetShadowMapResolution() const -> uint32_t override { return shadowMapResolution; }
		SH_GAME_API auto GetDirection() const -> const Vec3& { return direction; }
		SH_GAME_API auto GetIntensity() const -> float { return intensity; }

		/// @brief 디렉셔널 라이트의 광원 공간 변환 행렬 (proj * view)을 계산한다.
		/// @details 디렉셔널 라이트는 평행광이므로 직교 투영을 사용한다. 광원 위치는 씬 중심에서 광선
		///          반대 방향으로 farPlane * 0.5 만큼 이동한 가상 위치를 사용한다.
		///          현재 구현은 단일 셰도우 맵 기준이며, CSM은 후속 작업으로 남겨둔다.
		SH_GAME_API auto GetLightSpaceMatrix() const -> glm::mat4;

		SH_GAME_API auto GetShadowCamera() -> render::Camera*;
		SH_GAME_API auto GetShadowCamera() const -> const render::Camera*;
		SH_GAME_API auto GetShadowMap() const -> render::RenderTexture*;
	private:
		/// @brief bCastShadow=true 시점에 호출. RT를 생성/빌드하고 카메라를 직교투영으로 설정.
		void EnsureShadowResources();
		/// @brief 매 프레임 광원 방향에 맞춰 shadowCamera의 pos/lookAt을 갱신.
		void UpdateShadowCamera();
	private:
		PROPERTY(direction)
		Vec3 direction{ -1.f, -1.f, -1.f };
		PROPERTY(intensity)
		float intensity = 1.f;

		PROPERTY(bCastShadow)
		bool bCastShadow = false;
		PROPERTY(shadowBias)
		float shadowBias = 0.005f;
		PROPERTY(shadowMapResolution)
		uint32_t shadowMapResolution = 1024;
		PROPERTY(shadowOrthoSize)
		float shadowOrthoSize = 20.f;
		PROPERTY(shadowNearPlane)
		float shadowNearPlane = 0.1f;
		PROPERTY(shadowFarPlane)
		float shadowFarPlane = 100.f;

		PROPERTY(shadowMap, core::PropertyOption::invisible)
		render::RenderTexture* shadowMap = nullptr;

		render::Camera shadowCamera;
	};
}//namespace
