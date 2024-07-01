#pragma once

#include "Export.h"

#include "Core/NonCopyable.h"
#include <stdint.h>

namespace sh::render
{
	class Framebuffer : public core::INonCopyable
	{
	public:
		SH_RENDER_API virtual ~Framebuffer() = default;
		SH_RENDER_API virtual auto GetWidth() const -> uint32_t = 0;
		SH_RENDER_API virtual auto GetHeight() const -> uint32_t = 0;
	};
}