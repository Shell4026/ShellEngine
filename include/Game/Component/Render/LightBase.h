#pragma once
#include "Game/Export.h"
#include "Game/Component/Component.h"
#include "Game/ILight.h"

#include "Render/ShadowMapManager.h"

namespace sh::game
{
	/// @brief 광원 컴포넌트 추상 클래스
	class LightBase : public Component, public ILight, public render::IShadowCaster
	{
		SCLASS(LightBase)
	public:
		SH_GAME_API LightBase(GameObject& owner);

		SH_GAME_API void Awake() override;
		SH_GAME_API void OnDestroy() override;
		SH_GAME_API void OnEnable() override;
		SH_GAME_API void OnDisable() override;
		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& prop) override;

		SH_GAME_API void SetCastShadow(bool bCastShadow);

		/// @brief 그림자 아틀라스 텍스처를 반환한다 (모든 광원이 공유)
		SH_GAME_API auto GetShadowMap() const -> render::RenderTexture*;
		/// @brief 아틀라스 내에서 이 광원의 슬롯 정보를 반환한다.
		SH_GAME_API auto GetShadowSlot() const -> render::ShadowMapManager::Slot;

		virtual void SetIntensity(float intensity) = 0;
		virtual auto GetIntensity() const -> float = 0;
		virtual auto GetLightType() const -> Type = 0;
		virtual auto GetLightSpaceMatrix() const -> glm::mat4 = 0;

		virtual auto GetShadowBias() const -> float { return shadowBias; }
		virtual auto GetShadowMapResolution() const -> uint32_t { return shadowMapResolution; }
		virtual auto GetShadowViewMatrix() const -> glm::mat4 = 0;
		virtual auto GetShadowProjMatrix() const -> glm::mat4 = 0;
		virtual auto GetShadowPos() const -> glm::vec3 = 0;
		virtual auto GetShadowLookAt() const -> glm::vec3 = 0;

		void SetShadowBias(float bias) { shadowBias = bias; }
		void SetShadowMapResolution(uint32_t res) { shadowMapResolution = res; }
		auto IsCastShadow() const -> bool { return bCastShadow; }
		auto GetShadowOrthoSize() const -> float { return shadowOrthoSize; }
		auto GetShadowNearPlane() const -> float { return shadowNearPlane; }
		auto GetShadowFarPlane() const -> float { return shadowFarPlane; }
	protected:
		SH_GAME_API void UpdateLightOctree();
		SH_GAME_API void RegisterToShadowManager();
		SH_GAME_API void UnregisterFromShadowManager();
	private:
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
		PROPERTY(bCastShadow)
		bool bCastShadow = false;
		bool bRegistered = false;
	};
}//namespace