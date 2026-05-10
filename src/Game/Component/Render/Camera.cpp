#include "Component/Render/Camera.h"
#include "World.h"

#include "Render/Renderer.h"
#include "Render/IRenderContext.h"
#include "Render/RenderData.h"

#include "Physics/Ray.h"

#include "glm/gtc/matrix_transform.hpp"

#include <cstdint>
namespace sh::game
{
	Camera::Camera(GameObject& owner) :
		Component(owner),
		renderTexture(nullptr)
	{
		fovRadians = glm::radians(fov);
		owner.world.RegisterCamera(*this);
		renderData.renderViewers.resize(1);
		canPlayInEditor = true;
	}
	Camera::~Camera() = default;
	
	SH_GAME_API void Camera::BeginUpdate()
	{
		render::RenderViewer& viewer = renderData.renderViewers[0];
		if (renderTexture == nullptr)
		{
			const glm::vec2 size = gameObject.world.renderer.GetContext()->GetViewportEnd() - gameObject.world.renderer.GetContext()->GetViewportStart();
			width = size.x;
			height = size.y;
			viewer.viewportRect = { 0, 0, size.x, size.y };
			viewer.viewportScissor = { 0, 0, size.x, size.y };
			renderData.ClearRenderTargets();
		}
		else
		{
			const glm::vec2 size = renderTexture->GetSize();
			width = size.x;
			height = size.y;
			viewer.viewportRect = { 0, 0, size.x, size.y };
			viewer.viewportScissor = { 0, 0, size.x, size.y };

			renderData.SetRenderTarget(renderTexture);
		}
		gameObject.transform->UpdateMatrix();
		viewer.pos = gameObject.transform->GetWorldPosition();
		viewer.to = lookPos;
		UpdateViewMatrix();
		UpdateProjMatrix();

		world.renderer.PushRenderData(renderData);
	}
	SH_GAME_API void Camera::OnDestroy()
	{
		if (gameObject.world.GetMainCamera() == this)
		{
			gameObject.world.SetMainCamera(nullptr);
		}
		gameObject.world.UnRegisterCamera(*this);
		Super::OnDestroy();
	}
	SH_GAME_API auto Camera::ScreenPointToRay(const Vec2& mousePos) const -> phys::Ray
	{
		const float aspect = width / height;

		const float ndcX = 2.f * mousePos.x / width - 1.0f;
		const float ndcY = 1.0f - 2.f * mousePos.y / height;

		glm::vec4 camCoord{ 0.f, 0.f, 0.f, 1.f };

		camCoord.x = aspect * ndcX;
		camCoord.y = ndcY;
		camCoord.z = -1.f / glm::tan(fovRadians / 2.f);

		glm::vec3 worldCoord{ glm::inverse(GetViewMatrix()) * camCoord };
		glm::vec3 dir = glm::normalize(worldCoord - glm::vec3{ gameObject.transform->position });

		return phys::Ray(gameObject.transform->position, dir);
	}
	SH_GAME_API auto Camera::ScreenPointToRayOrtho(const Vec2& mousePos) const -> phys::Ray
	{
		const float ndcX = 2.f * (mousePos.x / width) - 1.f;
		const float ndcY = 1.f - 2.f * (mousePos.y / height);

		// Vulkan z_ndc = 0 ~ 1
		glm::vec4 ndcNear(ndcX, ndcY, 0.f, 1.f);
		glm::vec4 ndcFar(ndcX, ndcY, 1.f, 1.f);

		glm::mat4 invViewProj = glm::inverse(GetProjMatrix() * GetViewMatrix());

		glm::vec4 worldNear4 = invViewProj * ndcNear;
		glm::vec4 worldFar4 = invViewProj * ndcFar;

		glm::vec3 worldNear = glm::vec3(worldNear4) / worldNear4.w;
		glm::vec3 worldFar = glm::vec3(worldFar4) / worldFar4.w;

		glm::vec3 dir = glm::normalize(worldFar - worldNear);

		return phys::Ray(worldNear, dir);
	}
	SH_GAME_API void Camera::SetFov(float degree)
	{
		const float aspect = width / height;
		fov = degree;
		fovx = glm::degrees(glm::atan(glm::tan(glm::radians(fov) * 0.5f) * aspect) * 2.f);
		fovRadians = glm::radians(fov);
	}

	SH_GAME_API void Camera::UpdateViewMatrix()
	{
		renderData.renderViewers.front().viewMatrix = glm::lookAt(glm::vec3{ gameObject.transform->GetWorldPosition() }, glm::vec3{ lookPos }, glm::vec3{ up });
	}
	SH_GAME_API void Camera::UpdateProjMatrix()
	{
		if (projection == Projection::Perspective)
		{
			renderData.renderViewers.front().projMatrix = glm::perspectiveFovRH_ZO(fovRadians, width, height, nearPlane, farPlane);
		}
		else
		{
			const float dis = glm::length(glm::vec3{ lookPos } - glm::vec3{ gameObject.transform->GetWorldPosition() }) / 2.0f;
			const float aspect = width / height;
			renderData.renderViewers.front().projMatrix = glm::orthoRH_ZO(-dis * aspect, dis * aspect, -dis, dis, nearPlane, farPlane);
		}
	}
}//namespace
