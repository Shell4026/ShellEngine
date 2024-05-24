#pragma once

#include "Texture.h"

namespace sh::render
{
	class ITextureBuffer
	{
	public:
		virtual void Create(const VulkanRenderer& renderer, const void* data, uint32_t width, uint32_t height, Texture::TextureFormat format) = 0;
		virtual void Bind() = 0;
	};
}