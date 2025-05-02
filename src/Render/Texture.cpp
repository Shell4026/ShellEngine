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

			ITextureBuffer::CreateInfo ci{};
			ci.width = width;
			ci.height = height;
			ci.format = format;
			ci.aniso = aniso;
			ci.bGenerateMipmap = true;
			textureBuffer->Create(*context, ci);

			for (int m = 0; m < pixels.size(); ++m)
				textureBuffer->SetData(pixels[m].data(), m);
		}
		onBufferUpdate.Notify(this);
	}
	auto Texture::CheckSRGB() const -> bool
	{
		if (format == TextureFormat::SRGB24 || format == TextureFormat::SRGBA32)
			return true;
		return false;
	}

	Texture::Texture(TextureFormat format, uint32_t width, uint32_t height, bool bUseMipmap) :
		context(nullptr),
		format(format), width(width), height(height),
		aniso(1)
	{
		bSRGB = CheckSRGB();

		uint32_t mipLevels = bUseMipmap ? static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1 : 1;
		pixels.resize(mipLevels);
		int mipWidth = width;
		int mipHeight = height;
		for (int i = 0; i < mipLevels; ++i)
		{
			pixels[i].resize(mipWidth * mipHeight * 4);
			mipWidth = std::max(1, mipWidth / 2);
			mipHeight = std::max(1, mipHeight / 2);
		}
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

	SH_RENDER_API void Texture::SetPixelData(const std::vector<uint8_t>& pixels, uint32_t mipLevel)
	{
		assert(this->pixels[mipLevel].size() == pixels.size());
		this->pixels[mipLevel] = pixels;
		bSetDataDirty = true;
		SyncDirty();
	}

	SH_RENDER_API auto Texture::GetPixelData() const -> const std::vector<std::vector<Byte>>&
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
	SH_RENDER_API void sh::render::Texture::SetAnisoLevel(uint32_t aniso)
	{
		this->aniso = aniso;
		bSetDataDirty = true;
		SyncDirty();
	}
	SH_RENDER_API auto Texture::GetAnisoLevel() const -> uint32_t
	{
		return aniso;
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