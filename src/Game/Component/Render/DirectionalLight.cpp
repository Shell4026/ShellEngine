#include "Component/Render/DirectionalLight.h"

#include "World.h"

#include "Core/SObject.h"

#include "Render/Renderer.h"
#include "Render/ShadowMapManager.h"
#include "Render/RenderTexture.h"

#include <glm/gtc/matrix_transform.hpp>

namespace sh::game
{
	DirectionalLight::DirectionalLight(GameObject& owner) :
		Component(owner)
	{
		world.GetLightOctree().Insert(*this);
		canPlayInEditor = true;
	}
	DirectionalLight::~DirectionalLight() = default;

	SH_GAME_API void DirectionalLight::Awake()
	{
		Super::Awake();
		if (bCastShadow)
			RegisterToShadowManager();
	}

	SH_GAME_API void DirectionalLight::OnDestroy()
	{
		UnregisterFromShadowManager();
		world.GetLightOctree().Erase(*this);
		Super::OnDestroy();
	}

	SH_GAME_API void DirectionalLight::OnPropertyChanged(const core::reflection::Property& prop)
	{
		if (prop.GetName() == "bCastShadow")
		{
			if (bCastShadow)
				RegisterToShadowManager();
			else
				UnregisterFromShadowManager();
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
			RegisterToShadowManager();
		else
			UnregisterFromShadowManager();
	}
	SH_GAME_API void DirectionalLight::SetShadowBias(float bias)
	{
		shadowBias = bias;
	}
	SH_GAME_API void DirectionalLight::SetShadowMapResolution(uint32_t res)
	{
		shadowMapResolution = res;
	}
	SH_GAME_API auto DirectionalLight::GetLightSpaceMatrix() const -> glm::mat4
	{
		return GetShadowProjMatrix() * GetShadowViewMatrix();
	}
	SH_GAME_API auto DirectionalLight::GetShadowMap() const -> render::RenderTexture*
	{
		if (!bCastShadow)
			return nullptr;
		return world.GetShadowMapManager().GetAtlas();
	}
	SH_GAME_API auto DirectionalLight::GetShadowSlot() const -> render::ShadowMapManager::Slot
	{
		if (!bCastShadow)
			return render::ShadowMapManager::Slot{};
		return world.GetShadowMapManager().GetSlot(*this);
	}

	SH_GAME_API auto DirectionalLight::GetShadowViewMatrix() const -> glm::mat4
	{
		const glm::vec3 dir = glm::normalize(glm::vec3{ direction.x, direction.y, direction.z });
		const glm::vec3 sceneCenter{ 0.f, 0.f, 0.f };
		const float distance = shadowFarPlane * 0.5f;
		const glm::vec3 lightPos = sceneCenter - dir * distance;
		const glm::vec3 worldUp = (glm::abs(dir.y) > 0.99f) ? glm::vec3{ 0.f, 0.f, 1.f } : glm::vec3{ 0.f, 1.f, 0.f };
		return glm::lookAt(lightPos, lightPos + dir, worldUp);
	}
	SH_GAME_API auto DirectionalLight::GetShadowProjMatrix() const -> glm::mat4
	{
		return glm::orthoRH_ZO(-shadowOrthoSize, shadowOrthoSize, -shadowOrthoSize, shadowOrthoSize, shadowNearPlane, shadowFarPlane);
	}
	SH_GAME_API auto DirectionalLight::GetShadowPos() const -> glm::vec3
	{
		const glm::vec3 dir = glm::normalize(glm::vec3{ direction.x, direction.y, direction.z });
		const glm::vec3 sceneCenter{ 0.f, 0.f, 0.f };
		const float distance = shadowFarPlane * 0.5f;
		return sceneCenter - dir * distance;
	}
	SH_GAME_API auto DirectionalLight::GetShadowLookAt() const -> glm::vec3
	{
		return glm::vec3{ 0.f, 0.f, 0.f };
	}
	SH_GAME_API auto DirectionalLight::GetPos() const -> const Vec3&
	{
		return gameObject.transform->GetWorldPosition();
	}

	void DirectionalLight::RegisterToShadowManager()
	{
		if (bRegistered)
			return;
		world.GetShadowMapManager().Register(*this);
		bRegistered = true;
	}
	void DirectionalLight::UnregisterFromShadowManager()
	{
		if (!bRegistered)
			return;
		world.GetShadowMapManager().Unregister(*this);
		bRegistered = false;
	}
}//namespace
