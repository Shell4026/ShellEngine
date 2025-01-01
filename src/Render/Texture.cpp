#include "pch.h"
#include "Texture.h"
#include "VulkanContext.h"
#include "VulkanTextureBuffer.h"

#include "Core/ThreadSyncManager.h"

#include <cstring>

namespace sh::render
{
	Texture::Texture(TextureFormat format, uint32_t width, uint32_t height) :
		context(nullptr),
		format(format), width(width), height(height),
		bDirty(false)
	{
		pixels.resize(width * height * 4);
	}
	Texture::Texture(Texture&& other) noexcept :
		context(other.context),
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
		if (context != nullptr)
			Build(*context);
	}

	auto Texture::GetPixelData() const -> const std::vector<Byte>&
	{
		return pixels;
	}

	void Texture::Build(const IRenderContext& context)
	{
		this->context = &context;
		if (context.GetRenderAPIType()  == RenderAPI::Vulkan)
		{
			buffer[core::ThreadType::Game] = std::make_unique<vk::VulkanTextureBuffer>();
			buffer[core::ThreadType::Game]->Create(context, width, height, format);
			buffer[core::ThreadType::Game]->SetData(pixels.data());
			
			buffer[core::ThreadType::Render] = std::make_unique<vk::VulkanTextureBuffer>();
			buffer[core::ThreadType::Render]->Create(context, width, height, format);
			buffer[core::ThreadType::Render]->SetData(pixels.data());
		}

		SetDirty();
	}

	auto Texture::GetBuffer(core::ThreadType thr) const -> ITextureBuffer*
	{
		return buffer[thr].get();
	}

	void Texture::SetDirty()
	{
		if (bDirty)
			return;

		core::ThreadSyncManager::GetInstance()->PushSyncable(*this);

		bDirty = true;
	}
	void Texture::Sync()
	{
		std::swap(buffer[core::ThreadType::Render], buffer[core::ThreadType::Game]);

		bDirty = false;
	}
}