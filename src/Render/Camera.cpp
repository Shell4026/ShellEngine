#include "Camera.h"

#include "glm/gtc/matrix_transform.hpp"

#include <cassert>

namespace sh::render
{
	uint32_t Camera::nextId = 0;

	Camera::Camera() :
		id(nextId++),
		renderTexture(nullptr), priority(0)
	{
		fovRadians = glm::radians(60.0f);
		pos = glm::vec3(0.0f, 0.0f, 0.0f);
		to = glm::vec3(0.0f, 0.0f, 1.0f);
		up = glm::vec3(0.0f, 1.0f, 0.0f);
	}
	SH_RENDER_API void Camera::SetRenderTexture(RenderTexture* framebuffer)
	{
		this->renderTexture = framebuffer;
	}
	SH_RENDER_API auto Camera::GetRenderTexture() const -> RenderTexture*
	{
		return renderTexture;
	}

	SH_RENDER_API void Camera::SetPriority(int priority)
	{
		this->priority = priority;
	}
	SH_RENDER_API auto Camera::GetPriority() const -> int
	{
		return priority;
	}
	SH_RENDER_API void Camera::SetFov(float degree)
	{
		fovRadians = glm::radians(degree);
	}
	SH_RENDER_API auto Camera::GetFovRadian() const -> float
	{
		return fovRadians;
	}
	SH_RENDER_API auto Camera::GetProjMatrix() const -> const glm::mat4&
	{
		return bufferData.matProj;
	}
	SH_RENDER_API auto Camera::GetViewMatrix() const -> const glm::mat4&
	{
		return bufferData.matView;
	}
	SH_RENDER_API void Camera::SetPos(const glm::vec3& pos)
	{
		this->pos = pos;
	}
	SH_RENDER_API auto Camera::GetPos() const -> const glm::vec3&
	{
		return pos;
	}
	SH_RENDER_API void Camera::SetLookPos(const glm::vec3& pos)
	{
		this->to = pos;
	}
	SH_RENDER_API auto Camera::GetLookPos() const -> const glm::vec3&
	{
		return to;
	}
	SH_RENDER_API void Camera::SetUpVector(const glm::vec3& up)
	{
		this->up = up;
	}
	SH_RENDER_API auto Camera::GetUpVector() const -> const glm::vec3&
	{
		return up;
	}
	SH_RENDER_API void Camera::SetNearPlane(float near)
	{
		nearPlane = near;
	}
	SH_RENDER_API auto Camera::GetNearPlane() const -> float
	{
		return nearPlane;
	}
	SH_RENDER_API void Camera::SetFarPlane(float far)
	{
		farPlane = far;
	}
	SH_RENDER_API auto Camera::GetFarPlane() const -> float
	{
		return farPlane;
	}
	SH_RENDER_API void Camera::SetWidth(float width)
	{
		this->width = width;
	}
	SH_RENDER_API auto Camera::GetWidth() const -> float
	{
		return width;
	}
	SH_RENDER_API void Camera::SetHeight(float height)
	{
		this->height = height;
	}
	SH_RENDER_API auto Camera::GetHeight() const -> float
	{
		return height;
	}
	SH_RENDER_API void Camera::UpdateMatrix()
	{
		bufferData.matProj = glm::perspectiveFov(fovRadians, width, height, nearPlane, farPlane);
		bufferData.matView = glm::lookAt(pos, to, up);
	}
	SH_RENDER_API void Camera::SetRenderTagMask(uint32_t mask)
	{
		renderTagMask = mask;
	}
	SH_RENDER_API void Camera::AddRenderTagMask(uint32_t tagId)
	{
		renderTagMask |= tagId;
	}
	SH_RENDER_API void Camera::RemoveRenderTagMask(uint32_t tagId)
	{
		assert(renderTagMask & tagId);
		renderTagMask ^= tagId;
	}
	SH_RENDER_API auto Camera::GetRenderTagMask() const -> uint32_t
	{
		return renderTagMask;
	}
	SH_RENDER_API auto Camera::CheckRenderTag(uint32_t tagId) const -> bool
	{
		return renderTagMask & tagId;
	}
}//namespace render