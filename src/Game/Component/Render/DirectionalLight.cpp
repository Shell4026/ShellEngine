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
		LightBase(owner)
	{
	}
	DirectionalLight::~DirectionalLight() = default;

	SH_GAME_API bool DirectionalLight::Intersect(const render::AABB& aabb) const
	{
		// 항상 통과
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
	SH_GAME_API auto DirectionalLight::GetLightSpaceMatrix() const -> glm::mat4
	{
		return GetShadowProjMatrix() * GetShadowViewMatrix();
	}

	SH_GAME_API auto DirectionalLight::GetShadowViewMatrix() const -> glm::mat4
	{
		const glm::vec3 dir = glm::normalize(glm::vec3{ direction.x, direction.y, direction.z });
		const glm::vec3 sceneCenter{ 0.f, 0.f, 0.f };
		const float distance = GetShadowFarPlane() * 0.5f;
		const glm::vec3 lightPos = sceneCenter - dir * distance;
		const glm::vec3 worldUp = (glm::abs(dir.y) > 0.99f) ? glm::vec3{ 0.f, 0.f, 1.f } : glm::vec3{ 0.f, 1.f, 0.f };
		return glm::lookAt(lightPos, lightPos + dir, worldUp);
	}
	SH_GAME_API auto DirectionalLight::GetShadowProjMatrix() const -> glm::mat4
	{
		const float ortho = GetShadowOrthoSize();
		return glm::orthoRH_ZO(-ortho, ortho, -ortho, ortho, GetShadowNearPlane(), GetShadowFarPlane());
	}
	SH_GAME_API auto DirectionalLight::GetShadowPos() const -> glm::vec3
	{
		const glm::vec3 dir = glm::normalize(glm::vec3{ direction.x, direction.y, direction.z });
		const glm::vec3 sceneCenter{ 0.f, 0.f, 0.f };
		const float distance = GetShadowFarPlane() * 0.5f;
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
}//namespace
