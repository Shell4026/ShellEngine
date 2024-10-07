#include "pch.h"
#include "Texture.h"

#include "VulkanRenderer.h"
#include "VulkanTextureBuffer.h"

#include <cstring>

namespace sh::render
{
	Texture::Texture(TextureFormat format, uint32_t width, uint32_t height) :
		renderer(nullptr),
		format(format), width(width), height(height),
		bDirty(false)
	{
		pixels.resize(width * height * 4);
	}
	Texture::Texture(Texture&& other) noexcept :
		renderer(other.renderer),
		format(other.format), width(other.width), height(other.height),
		pixels(std::move(other.pixels)), buffer(std::move(other.buffer)),
		bDirty(other.bDirty)
	{
	}
	Texture::~Texture()
	{
	}

	void Texture::SetPixelData(void* data)
	{
		std::memcpy(pixels.data(), data, pixels.size());
		if (renderer)
			Build(*renderer);
	}

	auto Texture::GetPixelData() const -> const std::vector<Byte>&
	{
		return pixels;
	}

	void Texture::Build(Renderer& renderer)
	{
		this->renderer = &renderer;
		if (renderer.apiType == RenderAPI::Vulkan)
		{
			buffer[core::ThreadType::Game] = std::make_unique<VulkanTextureBuffer>();
			buffer[core::ThreadType::Game]->Create(static_cast<const VulkanRenderer&>(renderer), pixels.data(), width, height, format);
			buffer[core::ThreadType::Render] = std::make_unique<VulkanTextureBuffer>();
			buffer[core::ThreadType::Render]->Create(static_cast<const VulkanRenderer&>(renderer), pixels.data(), width, height, format);
		}

		SetDirty();
	}

	auto Texture::GetBuffer(core::ThreadType thr) -> ITextureBuffer*
	{
		return buffer[thr].get();
	}

	void Texture::SetDirty()
	{
		if (bDirty)
			return;

		bDirty = true;
		if (renderer != nullptr)
			renderer->GetThreadSyncManager().PushSyncable(*this);
	}
	void Texture::Sync()
	{
		std::swap(buffer[core::ThreadType::Render], buffer[core::ThreadType::Game]);

		bDirty = false;
	}
}