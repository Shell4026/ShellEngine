#include "Component/Render/DirectionalLight.h"

#include "World.h"
#include "GameRenderer.h"

#include "Core/SObject.h"

#include <glm/gtc/matrix_transform.hpp>

namespace sh::game
{
	DirectionalLight::DirectionalLight(GameObject& owner) :
		Component(owner)
	{
		world.GetLightOctree().Insert(*this);
		
		GameRenderer* const srPtr = dynamic_cast<GameRenderer*>(world.renderer.GetScriptableRenderer());
		if (srPtr != nullptr)
		{
			world.renderer.AddCamera(shadowCamera);
			srPtr->AddShadowCasterCamera(shadowCamera);
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
		if (bCastShadow && shadowMap != nullptr)
			UpdateShadowCamera();
	}

	SH_GAME_API void DirectionalLight::OnDestroy()
	{
		GameRenderer* const srPtr = dynamic_cast<GameRenderer*>(world.renderer.GetScriptableRenderer());
		if (srPtr != nullptr)
		{
			srPtr->RemoveShadowCasterCamera(shadowCamera);
			world.renderer.RemoveCamera(shadowCamera);
		}
		world.GetLightOctree().Erase(*this);
		Super::OnDestroy();
	}

	SH_GAME_API void DirectionalLight::OnPropertyChanged(const core::reflection::Property& prop)
	{
		if (prop.GetName() == "bCastShadow")
		{
			if (bCastShadow && shadowMap == nullptr)
				EnsureShadowResources();
			// bCastShadow=false로 바뀌어도 RT는 보존(토글 시 재할당 비용 회피).
			// 메모리 회수가 필요하면 별도 정리 API를 도입할 것.
		}
		else if (prop.GetName() == "shadowMapResolution")
		{
			if (shadowMap != nullptr)
				shadowMap->SetSize(shadowMapResolution, shadowMapResolution);
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
	SH_GAME_API auto DirectionalLight::GetPos() const -> const Vec3&
	{
		return gameObject.transform->GetWorldPosition();
	}

	SH_GAME_API auto DirectionalLight::GetLightType() const -> ILight::Type
	{
		return ILight::Type::Directional;
	}
	SH_GAME_API void DirectionalLight::SetCastShadow(bool b)
	{
		bCastShadow = b;
		if (bCastShadow && shadowMap == nullptr)
			EnsureShadowResources();
	}
	SH_GAME_API void DirectionalLight::SetShadowBias(float bias)
	{
		shadowBias = bias;
	}
	SH_GAME_API void DirectionalLight::SetShadowMapResolution(uint32_t res)
	{
		shadowMapResolution = res;
		if (shadowMap != nullptr)
			shadowMap->SetSize(res, res);
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

	SH_GAME_API auto DirectionalLight::GetShadowCamera() -> render::Camera*
	{
		return bCastShadow ? &shadowCamera : nullptr;
	}
	SH_GAME_API auto DirectionalLight::GetShadowCamera() const -> const render::Camera*
	{
		return bCastShadow ? &shadowCamera : nullptr;
	}
	SH_GAME_API auto DirectionalLight::GetShadowMap() const -> render::RenderTexture*
	{
		return bCastShadow ? shadowMap : nullptr;
	}

	void DirectionalLight::EnsureShadowResources()
	{
		if (shadowMap == nullptr)
		{
			render::RenderTargetLayout layout{};
			layout.format = render::TextureFormat::None;
			layout.depthFormat = render::TextureFormat::D32;
			layout.bUseMSAA = false;

			shadowMap = core::SObject::Create<render::RenderTexture>(layout);
			shadowMap->SetSize(shadowMapResolution, shadowMapResolution);
			shadowMap->Build(*world.renderer.GetContext());
			shadowMap->SetName("DirectionalShadowMap");
		}

		shadowCamera.SetOrthographic(true);
		shadowCamera.SetWidth(shadowOrthoSize * 2.f);
		shadowCamera.SetHeight(shadowOrthoSize * 2.f);
		shadowCamera.SetNearPlane(shadowNearPlane);
		shadowCamera.SetFarPlane(shadowFarPlane);
		shadowCamera.SetRenderTexture(shadowMap);

		UpdateShadowCamera();
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
