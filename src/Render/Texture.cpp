#include "Texture.h"

#include "VulkanRenderer.h"
#include "VulkanTextureBuffer.h"

#include <cstring>

namespace sh::render
{
	Texture::Texture(TextureFormat format, uint32_t width, uint32_t height) :
		format(format), width(width), height(height)
	{
		pixels.resize(width * height * 4);
	}
	Texture::Texture(Texture&& other) noexcept :
		format(other.format), width(other.width), height(other.height),
		pixels(std::move(other.pixels)), buffer(std::move(other.buffer))
	{
	}
	Texture::~Texture()
	{
	}

	void Texture::SetPixelData(void* data)
	{
		std::memcpy(pixels.data(), data, pixels.size());
	}

	auto Texture::GetPixelData() const -> const std::vector<Byte>&
	{
		return pixels;
	}

	void Texture::Build(const VulkanRenderer& renderer)
	{
		if (renderer.apiType == RenderAPI::Vulkan)
		{
			buffer = std::make_unique<VulkanTextureBuffer>();
			buffer->Create(renderer, pixels.data(), width, height, format);
		}
	}

	auto Texture::GetBuffer() -> ITextureBuffer*
	{
		return buffer.get();
	}
}