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
	class DirectionalLight : public Component, public ILight, public render::IShadowCaster
	{
		COMPONENT(DirectionalLight)
	public:
		SH_GAME_API DirectionalLight(GameObject& owner);
		SH_GAME_API ~DirectionalLight();

		SH_GAME_API void Awake() override;
		SH_GAME_API void OnDestroy() override;
		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& prop) override;

		SH_GAME_API bool Intersect(const render::AABB& aabb) const override;

		SH_GAME_API void SetIntensity(float intensity) override;
		SH_GAME_API void SetDirection(const Vec3& dir);
		SH_GAME_API void SetCastShadow(bool b) override;
		SH_GAME_API void SetShadowBias(float bias) override;
		SH_GAME_API void SetShadowMapResolution(uint32_t res) override;
		/// @brief 라이트의 광원 공간 변환 행렬 (proj * view)을 반환한다.
		SH_GAME_API auto GetLightSpaceMatrix() const -> glm::mat4 override;

		/// @brief 그림자 아틀라스 텍스처를 반환한다 (모든 광원이 공유)
		SH_GAME_API auto GetShadowMap() const -> render::RenderTexture*;
		/// @brief 아틀라스 내에서 이 광원의 슬롯 정보를 반환한다.
		SH_GAME_API auto GetShadowSlot() const -> render::ShadowMapManager::Slot;

		SH_GAME_API auto GetShadowMapResolution() const -> uint32_t override { return shadowMapResolution; }
		SH_GAME_API auto GetShadowViewMatrix() const -> glm::mat4 override;
		SH_GAME_API auto GetShadowProjMatrix() const -> glm::mat4 override;
		SH_GAME_API auto GetShadowPos() const -> glm::vec3 override;
		SH_GAME_API auto GetShadowLookAt() const -> glm::vec3 override;

		SH_GAME_API auto GetPos() const -> const Vec3& override;
		auto GetLightType() const -> ILight::Type override { return ILight::Type::Directional; }
		auto IsCastShadow() const -> bool override { return bCastShadow; }
		auto GetShadowBias() const -> float override { return shadowBias; }
		auto GetDirection() const -> const Vec3& { return direction; }
		auto GetIntensity() const -> float { return intensity; }
	private:
		/// @brief bCastShadow 변경 시 월드의 ShadowMapManager에 등록/해제한다.
		void RegisterToShadowManager();
		void UnregisterFromShadowManager();
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

		bool bRegistered = false;
	};
}//namespace
