#include "Texture.h"
#include "VulkanContext.h"
#include "VulkanTextureBuffer.h"

#include "Core/ThreadSyncManager.h"

#include <cstring>

namespace sh::render
{
	void Texture::CreateTextureBuffer(core::ThreadType thread)
	{
		if (thread == core::ThreadType::Game)
			onBufferUpdate.Notify(this);

		if (context->GetRenderAPIType() == RenderAPI::Vulkan)
		{
			textureBuffer[thread] = std::make_unique<vk::VulkanTextureBuffer>();
			textureBuffer[thread]->Create(*context, width, height, format);
			textureBuffer[thread]->SetData(pixels.data());
		}
	}
	auto Texture::CheckSRGB() const -> bool
	{
		if (format == TextureFormat::SRGB24 || format == TextureFormat::SRGBA32)
			return true;
		return false;
	}

	Texture::Texture(TextureFormat format, uint32_t width, uint32_t height) :
		context(nullptr),
		format(format), width(width), height(height),
		bDirty(false)
	{
		bSRGB = CheckSRGB();
		pixels.resize(width * height * 4);
	}
	Texture::Texture(Texture&& other) noexcept :
		context(other.context),
		format(other.format), width(other.width), height(other.height),
		pixels(std::move(other.pixels)), textureBuffer(std::move(other.textureBuffer)),
		onBufferUpdate(std::move(other.onBufferUpdate)),
		bSRGB(other.bSRGB),
		bDirty(other.bDirty.load(std::memory_order::memory_order_relaxed)), bFormatDirty(other.bFormatDirty)
	{
	}
	Texture::~Texture()
	{
	}

	SH_RENDER_API void Texture::SetPixelData(void* data)
	{
		std::memcpy(pixels.data(), data, pixels.size());
		if (context != nullptr)
			Build(*context);
	}

	SH_RENDER_API auto Texture::GetPixelData() const -> const std::vector<Byte>&
	{
		return pixels;
	}

	SH_RENDER_API void Texture::Build(const IRenderContext& context)
	{
		this->context = &context;
		CreateTextureBuffer(core::ThreadType::Game);
		CreateTextureBuffer(core::ThreadType::Render);
	}

	SH_RENDER_API auto Texture::GetTextureBuffer(core::ThreadType thr) const -> ITextureBuffer*
	{
		return textureBuffer[thr].get();
	}
	SH_RENDER_API auto Texture::GetTextureFormat() const -> TextureFormat
	{
		return format;
	}

	SH_RENDER_API void Texture::ChangeTextureFormat(TextureFormat target)
	{
		if (format == target)
			return;
		assert(core::ThreadSyncManager::IsMainThread());

		format = target;
		bSRGB = CheckSRGB();

		CreateTextureBuffer(core::ThreadType::Game);

		bFormatDirty = true;
		SyncDirty();
	}
	SH_RENDER_API auto Texture::IsSRGB() const -> bool
	{
		return bSRGB;
	}
	SH_RENDER_API void Texture::SyncDirty()
	{
		if (bDirty.load(std::memory_order::memory_order_acquire))
			return;

		core::ThreadSyncManager::PushSyncable(*this);

		bDirty.store(true, std::memory_order::memory_order_release);
	}
	SH_RENDER_API void Texture::Sync()
	{
		std::swap(textureBuffer[core::ThreadType::Render], textureBuffer[core::ThreadType::Game]);
		if (bFormatDirty)
		{
			CreateTextureBuffer(core::ThreadType::Game);
			bFormatDirty = false;
		}

		bDirty.store(false, std::memory_order::memory_order_relaxed);
	}
}