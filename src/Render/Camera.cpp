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
		if (dirtyMask & 1)
			std::swap(pos[core::ThreadType::Render], pos[core::ThreadType::Game]);
		if (dirtyMask & 2)
			std::swap(to[core::ThreadType::Render], to[core::ThreadType::Game]);
		if (dirtyMask & 4)
			std::swap(up[core::ThreadType::Render], up[core::ThreadType::Game]);
		if (dirtyMask & 8)
			std::swap(fovRadians[core::ThreadType::Render], fovRadians[core::ThreadType::Game]);
		if (dirtyMask & 16)
			std::swap(nearPlane[core::ThreadType::Render], nearPlane[core::ThreadType::Game]);
		if (dirtyMask & 32)
			std::swap(farPlane[core::ThreadType::Render], farPlane[core::ThreadType::Game]);
		if (dirtyMask & 64)
			std::swap(width[core::ThreadType::Render], width[core::ThreadType::Game]);
		if (dirtyMask & 128)
			std::swap(height[core::ThreadType::Render], height[core::ThreadType::Game]);
		if (dirtyMask & 256)
			std::swap(bufferData[core::ThreadType::Render], bufferData[core::ThreadType::Game]);
		dirtyMask = 0;
		bDirty = false;
	}
	Camera::Camera() :
		id(nextId++),
		renderTexture(nullptr), priority(0)
	{
		fovRadians[core::ThreadType::Game] = glm::radians(60.0f);
		pos[core::ThreadType::Game] = glm::vec3(0.0f, 0.0f, 0.0f);
		to[core::ThreadType::Game] = glm::vec3(0.0f, 0.0f, 1.0f);
		up[core::ThreadType::Game] = glm::vec3(0.0f, 1.0f, 0.0f);

		fovRadians[core::ThreadType::Render] = glm::radians(60.0f);
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
		dirtyMask |= 1;
		UpdateViewMatrix();
		SyncDirty();
	}
	SH_RENDER_API void Camera::SetLookPos(const glm::vec3& pos)
	{
		this->to[core::ThreadType::Game] = pos;
		dirtyMask |= 2;
		UpdateViewMatrix();
		SyncDirty();
	}
	SH_RENDER_API void Camera::SetUpVector(const glm::vec3& up)
	{
		this->up[core::ThreadType::Game] = up;
		dirtyMask |= 4;
		UpdateViewMatrix();
		SyncDirty();
	}
	SH_RENDER_API void sh::render::Camera::SetFovRadian(float rad)
	{
		fovRadians[core::ThreadType::Game] = rad;
		dirtyMask |= 8;
		UpdateProjMatrix();
		SyncDirty();
	}
	SH_RENDER_API void Camera::SetFov(float degree)
	{
		fovRadians[core::ThreadType::Game] = glm::radians(degree);
		dirtyMask |= 8;
		UpdateProjMatrix();
		SyncDirty();
	}
	SH_RENDER_API void Camera::SetNearPlane(float near)
	{
		nearPlane[core::ThreadType::Game] = near;
		dirtyMask |= 16;
		UpdateProjMatrix();
		SyncDirty();
	}
	SH_RENDER_API void Camera::SetFarPlane(float far)
	{
		farPlane[core::ThreadType::Game] = far;
		dirtyMask |= 32;
		UpdateProjMatrix();
		SyncDirty();
	}
	SH_RENDER_API void Camera::SetWidth(float width)
	{
		this->width[core::ThreadType::Game] = width;
		dirtyMask |= 64;
		UpdateProjMatrix();
		SyncDirty();
	}
	SH_RENDER_API void Camera::SetHeight(float height)
	{
		this->height[core::ThreadType::Game] = height;
		dirtyMask |= 128;
		UpdateProjMatrix();
		SyncDirty();
	}
	SH_RENDER_API void Camera::SetOrthographic(bool bEnable)
	{
		bPerspective = !bEnable;
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
	void Camera::UpdateProjMatrix()
	{
		if (bPerspective)
		{
			bufferData[core::ThreadType::Game].matProj =
				glm::perspectiveFovRH_ZO(
					fovRadians[core::ThreadType::Game],
					width[core::ThreadType::Game],
					height[core::ThreadType::Game],
					nearPlane[core::ThreadType::Game],
					farPlane[core::ThreadType::Game]);
		}
		else
		{
			const float dis = glm::length(to[core::ThreadType::Game] - pos[core::ThreadType::Game]) / 2.0f;
			const float aspect = width[core::ThreadType::Game] / height[core::ThreadType::Game];
			bufferData[core::ThreadType::Game].matProj =
				glm::orthoRH_ZO(-dis * aspect, dis * aspect, -dis, dis, nearPlane[core::ThreadType::Game], farPlane[core::ThreadType::Game]);
		}
		dirtyMask |= 256;
		SyncDirty();
	}
	void Camera::UpdateViewMatrix()
	{
		bufferData[core::ThreadType::Game].matView = glm::lookAt(pos[core::ThreadType::Game], to[core::ThreadType::Game], up[core::ThreadType::Game]);
		dirtyMask |= 256;
		SyncDirty();
	}
}//namespace render