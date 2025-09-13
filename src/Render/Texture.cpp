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
			ci.filtering = static_cast<uint32_t>(filtering);
			ci.bGenerateMipmap = bGenerateMipmap;
			textureBuffer->Create(*context, ci);

			const int mipLevel = bGenerateMipmap ? pixels.size() : 1;
			for (int m = 0; m < mipLevel; ++m)
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
		bGenerateMipmap = bUseMipmap;

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
		aniso(other.aniso),
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
		if (textureBuffer != nullptr)
		{
			bSetDataDirty = true;
			SyncDirty();
		}
	}
	SH_RENDER_API void Texture::SetPixelData(const uint8_t* pixels, std::size_t size, uint32_t mipLevel)
	{
		assert(this->pixels[mipLevel].size() == size);
		std::memcpy(this->pixels[mipLevel].data(), pixels, size);
		if (textureBuffer != nullptr)
		{
			bSetDataDirty = true;
			SyncDirty();
		}
	}

	SH_RENDER_API auto Texture::GetPixelData() const -> const std::vector<std::vector<Byte>>&
	{
		return pixels;
	}
	SH_RENDER_API auto Texture::GetPixelData(uint32_t mipLevel) const -> const std::vector<Byte>&
	{
		assert(pixels.size() > mipLevel);
		return pixels[mipLevel];
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
	SH_RENDER_API auto Texture::GetWidth() const -> uint32_t
	{
		return width;
	}
	SH_RENDER_API auto Texture::GetHeight() const -> uint32_t
	{
		return height;
	}

	SH_RENDER_API void Texture::ChangeTextureFormat(TextureFormat target)
	{
		if (format == target)
			return;

		format = target;
		bSRGB = CheckSRGB();
		if (textureBuffer != nullptr)
		{
			bSetDataDirty = true;

			SyncDirty();
		}
	}
	SH_RENDER_API auto Texture::IsSRGB() const -> bool
	{
		return bSRGB;
	}
	SH_RENDER_API auto Texture::GetMipLevel() const -> uint32_t
	{
		if (!bGenerateMipmap)
			return 1;
		return pixels.size();
	}
	SH_RENDER_API void sh::render::Texture::SetAnisoLevel(uint32_t aniso)
	{
		this->aniso = aniso;
		if (textureBuffer != nullptr)
		{
			bSetDataDirty = true;
			SyncDirty();
		}
	}
	SH_RENDER_API auto Texture::GetAnisoLevel() const -> uint32_t
	{
		return aniso;
	}
	SH_RENDER_API void Texture::SetGenerateMipmap(bool bGenerate)
	{
		bGenerateMipmap = bGenerate;
		bSetDataDirty = true;
		SyncDirty();
	}
	SH_RENDER_API auto Texture::IsGenerateMipmap() const -> bool
	{
		return bGenerateMipmap;
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
	SH_RENDER_API void Texture::SetSRGB(bool bSRGB)
	{
		if (bSRGB)
		{
			if (format == Texture::TextureFormat::RGB24)
				ChangeTextureFormat(Texture::TextureFormat::SRGB24);
			else if (format == Texture::TextureFormat::RGBA32)
				ChangeTextureFormat(Texture::TextureFormat::SRGBA32);
		}
		else
		{
			if (format == Texture::TextureFormat::SRGB24)
				ChangeTextureFormat(Texture::TextureFormat::RGB24);
			else if (format == Texture::TextureFormat::SRGBA32)
				ChangeTextureFormat(Texture::TextureFormat::RGBA32);
		}
	}
	SH_RENDER_API void Texture::SetFiltering(Filtering filter)
	{
		filtering = filter;
		if (textureBuffer != nullptr)
		{
			bSetDataDirty = true;
			SyncDirty();
		}
	}
	SH_RENDER_API auto Texture::GetFiltering() const -> Filtering
	{
		return filtering;
	}
	SH_RENDER_API void Texture::OnPropertyChanged(const core::reflection::Property& prop)
	{
		if (prop.GetName() == core::Util::ConstexprHash("aniso"))
		{
			SetAnisoLevel(aniso);
		}
		else if (prop.GetName() == core::Util::ConstexprHash("bSRGB"))
		{
			SetSRGB(bSRGB);
		}
		else if (prop.GetName() == core::Util::ConstexprHash("bGenerateMipmap"))
		{
			SetGenerateMipmap(bGenerateMipmap);
		}
		else if (prop.GetName() == core::Util::ConstexprHash("filtering"))
		{
			SetFiltering(filtering);
		}
	}
}