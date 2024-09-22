#pragma once

#include "Export.h"
#include "RenderTexture.h"

namespace sh::render
{
	class Camera 
	{
	private:
		RenderTexture* renderTexture;

		int priority;
	public:
		SH_RENDER_API Camera();

		SH_RENDER_API void SetRenderTexture(RenderTexture* framebuffer);
		SH_RENDER_API auto GetRenderTexture() -> RenderTexture*;

		SH_RENDER_API void SetPriority(int priority);
		SH_RENDER_API auto GetPriority() const -> int;
	};
} //namespace