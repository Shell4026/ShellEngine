#include "Component/Render/DirectionalLight.h"

#include "World.h"
#include "GameRenderer.h"

#include "Core/SObject.h"

#include "Render/Renderer.h"
#include "Render/ShadowMapManager.h"
#include "Render/RenderTexture.h"

#include <glm/gtc/matrix_transform.hpp>
#include <limits>

namespace sh::game
{
	DirectionalLight::DirectionalLight(GameObject& owner) :
		Component(owner)
	{
		world.GetLightOctree().Insert(*this);

		renderData.priority = 1000;

		GameRenderer* const srPtr = dynamic_cast<GameRenderer*>(world.renderer.GetScriptableRenderer());
		if (srPtr != nullptr)
		{
			//world.renderer.AddCamera(shadowCamera);
			//srPtr->AddShadowCasterCamera(shadowCamera);
		}
		else
		{
			SH_ERROR("ScriptableRenderer is not GameRenderer!");
		}

		canPlayInEditor = true;
	}
	DirectionalLight::~DirectionalLight() = default;

	SH_GAME_API void DirectionalLight::Awake()
	{
		Super::Awake();
		if (bCastShadow)
			EnsureShadowResources();
	}

	SH_GAME_API void DirectionalLight::BeginUpdate()
	{
		Super::BeginUpdate();
		if (bCastShadow && shadowSlot != render::ShadowMapManager::InvalidSlot)
			UpdateShadowCamera();
	}

	SH_GAME_API void DirectionalLight::OnDestroy()
	{
		ReleaseShadowResources();

		GameRenderer* const srPtr = dynamic_cast<GameRenderer*>(world.renderer.GetScriptableRenderer());
		if (srPtr != nullptr)
		{
			//srPtr->RemoveShadowCasterCamera(shadowCamera);
			//world.renderer.RemoveCamera(shadowCamera);
		}
		world.GetLightOctree().Erase(*this);
		Super::OnDestroy();
	}

	SH_GAME_API void DirectionalLight::OnPropertyChanged(const core::reflection::Property& prop)
	{
		if (prop.GetName() == "bCastShadow")
		{
			if (bCastShadow)
			{
				if (shadowSlot == render::ShadowMapManager::InvalidSlot)
					EnsureShadowResources();
			}
			else
			{
				ReleaseShadowResources();
			}
		}
		else if (prop.GetName() == "shadowMapResolution")
		{
			if (bCastShadow)
			{
				// 슬롯 크기는 변경 불가. 해제 후 재할당으로 새 해상도 슬롯을 받는다.
				ReleaseShadowResources();
				EnsureShadowResources();
			}
		}
	}

	SH_GAME_API bool DirectionalLight::Intersect(const render::AABB& aabb) const
	{
		return true;
	}
	SH_GAME_API void DirectionalLight::SetIntensity(float intensity)
	{
		this->intensity = intensity;
	}
	SH_GAME_API void DirectionalLight::SetDirection(const Vec3& dir)
	{
		direction = dir;
	}
	SH_GAME_API void DirectionalLight::SetCastShadow(bool b)
	{
		if (bCastShadow == b)
			return;
		bCastShadow = b;
		if (bCastShadow)
			EnsureShadowResources();
		else
			ReleaseShadowResources();
	}
	SH_GAME_API void DirectionalLight::SetShadowBias(float bias)
	{
		shadowBias = bias;
	}
	SH_GAME_API void DirectionalLight::SetShadowMapResolution(uint32_t res)
	{
		if (shadowMapResolution == res)
			return;
		shadowMapResolution = res;
		if (bCastShadow)
		{
			ReleaseShadowResources();
			EnsureShadowResources();
		}
	}

	SH_GAME_API auto DirectionalLight::GetLightSpaceMatrix() const -> glm::mat4
	{
		const glm::vec3 dir = glm::normalize(glm::vec3{ direction.x, direction.y, direction.z });
		const glm::vec3 sceneCenter = glm::vec3{ 0.f, 0.f, 0.f };
		const float distance = shadowFarPlane * 0.5f;
		const glm::vec3 lightPos = sceneCenter - dir * distance;

		const glm::vec3 worldUp = (glm::abs(dir.y) > 0.99f) ? glm::vec3{ 0.f, 0.f, 1.f } : glm::vec3{ 0.f, 1.f, 0.f };

		const glm::mat4 view = glm::lookAt(lightPos, lightPos + dir, worldUp);
		const glm::mat4 proj = glm::ortho(-shadowOrthoSize, shadowOrthoSize, -shadowOrthoSize, shadowOrthoSize, shadowNearPlane, shadowFarPlane);
		return proj * view;
	}
	SH_GAME_API auto DirectionalLight::GetShadowMap() const -> render::RenderTexture*
	{
		if (!bCastShadow)
			return nullptr;
		return render::ShadowMapManager::GetInstance()->GetAtlas();
	}
	SH_GAME_API auto DirectionalLight::GetShadowSlot() const -> render::ShadowMapManager::Slot
	{
		if (!bCastShadow || shadowSlot == render::ShadowMapManager::InvalidSlot)
			return render::ShadowMapManager::Slot{};
		return render::ShadowMapManager::GetInstance()->GetSlot(shadowSlot);
	}

	void DirectionalLight::EnsureShadowResources()
	{
		render::ShadowMapManager* mgr = render::ShadowMapManager::GetInstance();

		if (shadowSlot == render::ShadowMapManager::InvalidSlot)
		{
			shadowSlot = mgr->AllocateSlot(shadowMapResolution);
			if (shadowSlot == render::ShadowMapManager::InvalidSlot)
			{
				SH_ERROR("DirectionalLight: failed to allocate shadow slot");
				return;
			}
			mgr->RegisterCameraSlot(&shadowCamera, shadowSlot);
		}

		render::RenderTexture* atlas = mgr->GetAtlas();

		shadowCamera.SetOrthographic(true);
		shadowCamera.SetWidth(shadowOrthoSize * 2.f);
		shadowCamera.SetHeight(shadowOrthoSize * 2.f);
		shadowCamera.SetNearPlane(shadowNearPlane);
		shadowCamera.SetFarPlane(shadowFarPlane);
		shadowCamera.SetRenderTexture(atlas);

		UpdateShadowCamera();
	}

	void DirectionalLight::ReleaseShadowResources()
	{
		if (shadowSlot == render::ShadowMapManager::InvalidSlot)
			return;
		render::ShadowMapManager* mgr = render::ShadowMapManager::GetInstance();
		mgr->UnregisterCameraSlot(&shadowCamera);
		mgr->ReleaseSlot(shadowSlot);
		shadowSlot = render::ShadowMapManager::InvalidSlot;
		shadowCamera.SetRenderTexture(nullptr);
	}

	void DirectionalLight::UpdateShadowCamera()
	{
		const glm::vec3 dir = glm::normalize(glm::vec3{ direction.x, direction.y, direction.z });
		const glm::vec3 sceneCenter = glm::vec3{ 0.f, 0.f, 0.f };
		const float distance = shadowFarPlane * 0.5f;
		const glm::vec3 lightPos = sceneCenter - dir * distance;
		const glm::vec3 worldUp = (glm::abs(dir.y) > 0.99f) ? glm::vec3{ 0.f, 0.f, 1.f } : glm::vec3{ 0.f, 1.f, 0.f };

		shadowCamera.SetPos(lightPos);
		shadowCamera.SetLookPos(sceneCenter);
		shadowCamera.SetUpVector(worldUp);
	}
}//namespace
