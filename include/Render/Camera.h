#pragma once
#include "Export.h"
#include "IRenderContext.h"
#include "RenderTexture.h"

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
	class Camera 
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

		glm::vec3 pos;
		glm::vec3 to;
		glm::vec3 up;

		BufferData bufferData;

		float fovRadians = 0.0f;
		float nearPlane = 0.1f;
		float farPlane = 1000.f;
		float width = 0.f;
		float height = 0.f;
		int priority;
		uint32_t renderTagMask = 1;
	public:
		SH_RENDER_API Camera();

		SH_RENDER_API void SetRenderTexture(RenderTexture* framebuffer);
		SH_RENDER_API auto GetRenderTexture() const -> RenderTexture*;

		SH_RENDER_API void SetPriority(int priority);
		SH_RENDER_API auto GetPriority() const -> int;

		SH_RENDER_API void SetFovRadian(float rad);
		SH_RENDER_API void SetFov(float degree);
		SH_RENDER_API auto GetFovRadian() const -> float;

		SH_RENDER_API auto GetProjMatrix() const -> const glm::mat4&;
		SH_RENDER_API auto GetViewMatrix() const -> const glm::mat4&;

		SH_RENDER_API void SetPos(const glm::vec3& pos);
		SH_RENDER_API auto GetPos() const -> const glm::vec3&;
		SH_RENDER_API void SetLookPos(const glm::vec3& pos);
		SH_RENDER_API auto GetLookPos() const -> const glm::vec3&;
		SH_RENDER_API void SetUpVector(const glm::vec3& up);
		SH_RENDER_API auto GetUpVector() const -> const glm::vec3&;
		SH_RENDER_API void SetNearPlane(float near);
		SH_RENDER_API auto GetNearPlane() const -> float;
		SH_RENDER_API void SetFarPlane(float far);
		SH_RENDER_API auto GetFarPlane() const -> float;
		SH_RENDER_API void SetWidth(float width);
		SH_RENDER_API auto GetWidth() const -> float;
		SH_RENDER_API void SetHeight(float height);
		SH_RENDER_API auto GetHeight() const -> float;

		SH_RENDER_API void UpdateMatrix();

		SH_RENDER_API void SetRenderTagMask(uint32_t mask);
		SH_RENDER_API void AddRenderTagMask(uint32_t tagId);
		SH_RENDER_API void RemoveRenderTagMask(uint32_t tagId);
		SH_RENDER_API auto GetRenderTagMask() const -> uint32_t;
		/// @brief 렌더 태그가 mask에 통과 하는지 검증하는 함수.
		/// @param tagId 태그 id
		/// @return 통과 하면 true 아니면 false
		SH_RENDER_API auto CheckRenderTag(uint32_t tagId) const -> bool;
	};
} //namespace