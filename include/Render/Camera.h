#pragma once
#include "Export.h"
#include "IRenderContext.h"
#include "RenderTexture.h"

#include "Core/ISyncable.h"

#include <glm/mat4x4.hpp>

#include <stdint.h>

#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif

namespace sh::render
{
	/// @brief 렌더러용 카메라.
	class Camera : public core::ISyncable
	{
	public:
		struct alignas(16) BufferData
		{
			glm::mat4 matView;
			glm::mat4 matProj;
		};
		const uint32_t id;
	private:
		SH_RENDER_API static uint32_t nextId;
		RenderTexture* renderTexture;

		core::SyncArray<glm::vec3> pos;
		core::SyncArray<glm::vec3> to;
		core::SyncArray<glm::vec3> up;

		core::SyncArray<BufferData> bufferData;

		float fovRadians = 0.0f;
		core::SyncArray<float> nearPlane = { 0.1f, 0.1f };
		core::SyncArray<float> farPlane = { 1000.f, 1000.f };
		core::SyncArray<float> width = { 0.f, 0.f };
		core::SyncArray<float> height = { 0.f, 0.f };
		int priority;
		uint32_t renderTagMask = 1;

		bool bActive = true;
		bool bDirty = false;
	protected:
		SH_RENDER_API void SyncDirty() override;
		SH_RENDER_API void Sync() override;
	public:
		SH_RENDER_API Camera();

		SH_RENDER_API void SetRenderTexture(RenderTexture* framebuffer);
		SH_RENDER_API auto GetRenderTexture() const -> RenderTexture*;

		SH_RENDER_API void SetPriority(int priority);
		SH_RENDER_API auto GetPriority() const -> int;

		SH_RENDER_API void SetFovRadian(float rad);
		SH_RENDER_API void SetFov(float degree);
		SH_RENDER_API auto GetFovRadian() const -> float;

		SH_RENDER_API auto GetProjMatrix(core::ThreadType thr) const -> const glm::mat4&;
		SH_RENDER_API auto GetViewMatrix(core::ThreadType thr) const -> const glm::mat4&;

		SH_RENDER_API void SetPos(const glm::vec3& pos);
		SH_RENDER_API auto GetPos(core::ThreadType thr) const -> const glm::vec3&;
		SH_RENDER_API void SetLookPos(const glm::vec3& pos);
		SH_RENDER_API auto GetLookPos(core::ThreadType thr) const -> const glm::vec3&;
		SH_RENDER_API void SetUpVector(const glm::vec3& up);
		SH_RENDER_API auto GetUpVector(core::ThreadType thr) const -> const glm::vec3&;
		SH_RENDER_API void SetNearPlane(float near);
		SH_RENDER_API auto GetNearPlane(core::ThreadType thr) const -> float;
		SH_RENDER_API void SetFarPlane(float far);
		SH_RENDER_API auto GetFarPlane(core::ThreadType thr) const -> float;
		SH_RENDER_API void SetWidth(float width);
		SH_RENDER_API auto GetWidth(core::ThreadType thr) const -> float;
		SH_RENDER_API void SetHeight(float height);
		SH_RENDER_API auto GetHeight(core::ThreadType thr) const -> float;

		SH_RENDER_API void UpdateMatrix();

		SH_RENDER_API void SetRenderTagMask(uint32_t mask);
		SH_RENDER_API void AddRenderTagMask(uint32_t tagId);
		SH_RENDER_API void RemoveRenderTagMask(uint32_t tagId);
		SH_RENDER_API auto GetRenderTagMask() const -> uint32_t;
		/// @brief 렌더 태그가 mask에 통과 하는지 검증하는 함수.
		/// @param tagId 태그 id
		/// @return 통과 하면 true 아니면 false
		SH_RENDER_API auto CheckRenderTag(uint32_t tagId) const -> bool;

		SH_RENDER_API void SetActive(bool bActive);
		SH_RENDER_API auto GetActive() const -> bool;
	};
} //namespace