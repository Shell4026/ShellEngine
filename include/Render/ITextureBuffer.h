﻿#pragma once

#include "Export.h"
#include "Texture.h"

#include <cstdint>

namespace sh::render
{
	class Renderer;
	class Framebuffer;

	class ITextureBuffer
	{
	public:
		SH_RENDER_API virtual ~ITextureBuffer() = default;
		SH_RENDER_API virtual void Create(const Renderer& renderer, const void* data, uint32_t width, uint32_t height, Texture::TextureFormat format) = 0;
		SH_RENDER_API virtual void Create(const Framebuffer& framebuffer) = 0;
		SH_RENDER_API virtual void Clean() = 0;
	};
}