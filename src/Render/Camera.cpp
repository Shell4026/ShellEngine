#include "Camera.h"

#include "Core/ThreadSyncManager.h"

#include "glm/gtc/matrix_transform.hpp"

#include <cassert>

namespace sh::render
{
	uint32_t Camera::nextId = 0;

	SH_RENDER_API void Camera::SyncDirty()
	{
		if (bDirty)
			return;
		bDirty = true;
		
		core::ThreadSyncManager::PushSyncable(*this);
	}
	SH_RENDER_API void Camera::Sync()
	{
		std::swap(pos[core::ThreadType::Game], pos[core::ThreadType::Render]);
		std::swap(to[core::ThreadType::Game], to[core::ThreadType::Render]);
		std::swap(up[core::ThreadType::Game], up[core::ThreadType::Render]);
		std::swap(nearPlane[core::ThreadType::Game], nearPlane[core::ThreadType::Render]);
		std::swap(farPlane[core::ThreadType::Game], farPlane[core::ThreadType::Render]);
		std::swap(width[core::ThreadType::Game], width[core::ThreadType::Render]);
		std::swap(height[core::ThreadType::Game], height[core::ThreadType::Render]);
		std::swap(bufferData[core::ThreadType::Game], bufferData[core::ThreadType::Render]);
		bDirty = false;
	}
	Camera::Camera() :
		id(nextId++),
		renderTexture(nullptr), priority(0)
	{
		fovRadians = glm::radians(60.0f);
		pos[core::ThreadType::Game] = glm::vec3(0.0f, 0.0f, 0.0f);
		to[core::ThreadType::Game] = glm::vec3(0.0f, 0.0f, 1.0f);
		up[core::ThreadType::Game] = glm::vec3(0.0f, 1.0f, 0.0f);

		pos[core::ThreadType::Render] = glm::vec3(0.0f, 0.0f, 0.0f);
		to[core::ThreadType::Render] = glm::vec3(0.0f, 0.0f, 1.0f);
		up[core::ThreadType::Render] = glm::vec3(0.0f, 1.0f, 0.0f);
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
	SH_RENDER_API void sh::render::Camera::SetFovRadian(float rad)
	{
		fovRadians = rad;
	}
	SH_RENDER_API void Camera::SetFov(float degree)
	{
		fovRadians = glm::radians(degree);
	}
	SH_RENDER_API auto Camera::GetFovRadian() const -> float
	{
		return fovRadians;
	}
	SH_RENDER_API auto Camera::GetProjMatrix(core::ThreadType thr) const -> const glm::mat4&
	{
		return bufferData[thr].matProj;
	}
	SH_RENDER_API auto Camera::GetViewMatrix(core::ThreadType thr) const -> const glm::mat4&
	{
		return bufferData[thr].matView;
	}
	SH_RENDER_API void Camera::SetPos(const glm::vec3& pos)
	{
		this->pos[core::ThreadType::Game] = pos;
	}
	SH_RENDER_API auto Camera::GetPos(core::ThreadType thr) const -> const glm::vec3&
	{
		return pos[thr];
	}
	SH_RENDER_API void Camera::SetLookPos(const glm::vec3& pos)
	{
		this->to[core::ThreadType::Game] = pos;
	}
	SH_RENDER_API auto Camera::GetLookPos(core::ThreadType thr) const -> const glm::vec3&
	{
		return to[thr];
	}
	SH_RENDER_API void Camera::SetUpVector(const glm::vec3& up)
	{
		this->up[core::ThreadType::Game] = up;
	}
	SH_RENDER_API auto Camera::GetUpVector(core::ThreadType thr) const -> const glm::vec3&
	{
		return up[thr];
	}
	SH_RENDER_API void Camera::SetNearPlane(float near)
	{
		nearPlane[core::ThreadType::Game] = near;
	}
	SH_RENDER_API auto Camera::GetNearPlane(core::ThreadType thr) const -> float
	{
		return nearPlane[thr];
	}
	SH_RENDER_API void Camera::SetFarPlane(float far)
	{
		farPlane[core::ThreadType::Game] = far;
	}
	SH_RENDER_API auto Camera::GetFarPlane(core::ThreadType thr) const -> float
	{
		return farPlane[thr];
	}
	SH_RENDER_API void Camera::SetWidth(float width)
	{
		this->width[core::ThreadType::Game] = width;
	}
	SH_RENDER_API auto Camera::GetWidth(core::ThreadType thr) const -> float
	{
		return width[thr];
	}
	SH_RENDER_API void Camera::SetHeight(float height)
	{
		this->height[core::ThreadType::Game] = height;
	}
	SH_RENDER_API auto Camera::GetHeight(core::ThreadType thr) const -> float
	{
		return height[thr];
	}
	SH_RENDER_API void Camera::SetOrthographic(bool bEnable)
	{
		bPerspective = !bEnable;
	}
	SH_RENDER_API void Camera::UpdateMatrix()
	{
		if (bPerspective)
			bufferData[core::ThreadType::Game].matProj =
				glm::perspectiveFovRH_ZO(fovRadians, width[core::ThreadType::Game], height[core::ThreadType::Game], nearPlane[core::ThreadType::Game], farPlane[core::ThreadType::Game]);
		else
		{
			const float dis = glm::length(to[core::ThreadType::Game] - pos[core::ThreadType::Game]) / 2.0f;
			const float aspect = width[core::ThreadType::Game] / height[core::ThreadType::Game];
			bufferData[core::ThreadType::Game].matProj =
				glm::orthoRH_ZO(-dis * aspect, dis * aspect, -dis, dis, nearPlane[core::ThreadType::Game], farPlane[core::ThreadType::Game]);
		}
		bufferData[core::ThreadType::Game].matView = glm::lookAt(pos[core::ThreadType::Game], to[core::ThreadType::Game], up[core::ThreadType::Game]);
		SyncDirty();
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
	SH_RENDER_API void Camera::SetActive(bool bActive)
	{
		this->bActive = bActive;
	}
	SH_RENDER_API auto Camera::GetActive() const -> bool
	{
		return bActive;
	}
}//namespace render