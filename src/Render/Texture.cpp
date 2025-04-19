#include "Texture.h"
#include "VulkanContext.h"
#include "VulkanTextureBuffer.h"

#include "Core/ThreadSyncManager.h"

#include <cstring>

namespace sh::render
{
	void Texture::CreateTextureBuffer()
	{
		if (context->GetRenderAPIType() == RenderAPI::Vulkan)
		{
			if (textureBuffer == nullptr)
				textureBuffer = std::make_unique<vk::VulkanTextureBuffer>();
			textureBuffer->Create(*context, width, height, format);
			textureBuffer->SetData(pixels.data());
		}
		onBufferUpdate.Notify(this);
	}
	auto Texture::CheckSRGB() const -> bool
	{
		if (format == TextureFormat::SRGB24 || format == TextureFormat::SRGBA32)
			return true;
		return false;
	}

	Texture::Texture(TextureFormat format, uint32_t width, uint32_t height) :
		context(nullptr),
		format(format), width(width), height(height)
	{
		bSRGB = CheckSRGB();
		pixels.resize(width * height * 4);
	}
	Texture::Texture(Texture&& other) noexcept :
		context(other.context),
		format(other.format), width(other.width), height(other.height),
		pixels(std::move(other.pixels)), textureBuffer(std::move(other.textureBuffer)),
		onBufferUpdate(std::move(other.onBufferUpdate)),
		bSRGB(other.bSRGB), bSetDataDirty(other.bSetDataDirty)
	{
		if (other.bDirty.test_and_set(std::memory_order::memory_order_acquire))
			bDirty.test_and_set(std::memory_order::memory_order_relaxed);
	}
	Texture::~Texture()
	{
	}

	SH_RENDER_API void Texture::SetPixelData(void* data)
	{
		std::memcpy(pixels.data(), data, pixels.size());
		bSetDataDirty = true;
		SyncDirty();
	}

	SH_RENDER_API auto Texture::GetPixelData() const -> const std::vector<Byte>&
	{
		return pixels;
	}

	SH_RENDER_API void Texture::Build(const IRenderContext& context)
	{
		if (textureBuffer == nullptr)
		{
			this->context = &context;
			CreateTextureBuffer();
		}
	}

	SH_RENDER_API auto Texture::GetTextureBuffer() const -> ITextureBuffer*
	{
		return textureBuffer.get();
	}
	SH_RENDER_API auto Texture::GetTextureFormat() const -> TextureFormat
	{
		return format;
	}

	SH_RENDER_API void Texture::ChangeTextureFormat(TextureFormat target)
	{
		if (format == target)
			return;

		format = target;
		bSRGB = CheckSRGB();
		bSetDataDirty = true;

		SyncDirty();
	}
	SH_RENDER_API auto Texture::IsSRGB() const -> bool
	{
		return bSRGB;
	}
	SH_RENDER_API void Texture::SyncDirty()
	{
		if (!bDirty.test_and_set(std::memory_order::memory_order_acquire))
			core::ThreadSyncManager::PushSyncable(*this);
	}
	SH_RENDER_API void Texture::Sync()
	{
		if (bSetDataDirty)
		{
			CreateTextureBuffer();
			bSetDataDirty = false;
		}

		bDirty.clear(std::memory_order::memory_order_relaxed);
	}
}