#pragma once

#include "Export.h"
#include "RenderTexture.h"

#include <stdint.h>

namespace sh::render
{
	/// @brief 렌더러용 카메라. 실제 카메라 연산은 수행 하지 않는다.
	class Camera 
	{
	private:
		static uint32_t nextId;
		RenderTexture* renderTexture;

		int priority;
	public:
		const uint32_t id;
	public:
		SH_RENDER_API Camera();

		SH_RENDER_API void SetRenderTexture(RenderTexture* framebuffer);
		SH_RENDER_API auto GetRenderTexture() -> RenderTexture*;

		SH_RENDER_API void SetPriority(int priority);
		SH_RENDER_API auto GetPriority() const -> int;
	};
} //namespace