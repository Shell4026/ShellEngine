#pragma once
#include "Game/Export.h"
#include "Game/ILight.h"
#include "Game/Component/Component.h"

#include "Render/RenderData.h"
#include "Render/ShadowMapManager.h"

#include <glm/mat4x4.hpp>

namespace sh::render
{
	class RenderTexture;
}

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

		/// @brief 디렉셔널 라이트의 광원 공간 변환 행렬 (proj * view)을 계산한다.
		SH_GAME_API auto GetLightSpaceMatrix() const -> glm::mat4;
		/// @brief 그림자 아틀라스 텍스처를 반환한다 (모든 광원이 공유)
		SH_GAME_API auto GetShadowMap() const -> render::RenderTexture*;
		/// @brief 아틀라스 내에서 이 광원의 슬롯 정보를 반환한다.
		SH_GAME_API auto GetShadowSlot() const -> render::ShadowMapManager::Slot;

		auto GetPos() const -> const Vec3 & override { return gameObject.transform->GetWorldPosition(); }
		auto GetLightType() const -> ILight::Type override { return ILight::Type::Directional; }
		auto IsCastShadow() const -> bool override { return bCastShadow; }
		auto GetShadowBias() const -> float override { return shadowBias; }
		auto GetShadowMapResolution() const -> uint32_t override { return shadowMapResolution; }
		auto GetDirection() const -> const Vec3& { return direction; }
		auto GetIntensity() const -> float { return intensity; }
	private:
		/// @brief bCastShadow=true 시점에 호출. 매니저로부터 슬롯을 할당받고 카메라를 설정.
		void EnsureShadowResources();
		/// @brief 슬롯을 매니저에 반납하고 카메라 매핑을 해제.
		void ReleaseShadowResources();
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

		render::RenderData renderData;
		render::ShadowMapManager::SlotHandle shadowSlot = render::ShadowMapManager::InvalidSlot;
	};
}//namespace
