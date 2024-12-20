﻿#include "pch.h"
#include "RenderTexture.h"

#include "Renderer.h"
#include "VulkanRenderer.h"
#include "VulkanTextureBuffer.h"
#include "VulkanImpl/VulkanFramebuffer.h"

namespace sh::render
{
	RenderTexture::RenderTexture(Texture::TextureFormat format) :
		Texture(format, 1024, 1024),

		width(1024), height(1024)
	{
	}
	RenderTexture::~RenderTexture()
	{
	}

	RenderTexture::RenderTexture(RenderTexture&& other) noexcept :
		Texture(std::move(other)),

		width(other.width), height(other.height),
		framebuffer(std::move(other.framebuffer))
	{

	}

	SH_RENDER_API void RenderTexture::SetReadUsage(bool bReadUsage) noexcept
	{
		this->bReadUsage = bReadUsage;
	}
	SH_RENDER_API void RenderTexture::Build(Renderer& renderer)
	{
		this->renderer = &renderer;

		if (renderer.apiType == RenderAPI::Vulkan)
		{
			auto& vkRenderer = static_cast<const VulkanRenderer&>(renderer);
			framebuffer[core::ThreadType::Game] = std::make_unique<impl::VulkanFramebuffer>(vkRenderer.GetDevice(), vkRenderer.GetGPU(), vkRenderer.GetAllocator());
			framebuffer[core::ThreadType::Render] = std::make_unique<impl::VulkanFramebuffer>(vkRenderer.GetDevice(), vkRenderer.GetGPU(), vkRenderer.GetAllocator());

			VkFormat format = VkFormat::VK_FORMAT_R8G8B8A8_SRGB;
			switch (this->format)
			{
			case Texture::TextureFormat::SRGB24:
				format = VkFormat::VK_FORMAT_R8G8B8_SRGB;
				break;
			case Texture::TextureFormat::RGB24:
				format = VkFormat::VK_FORMAT_R8G8B8_UNORM;
				break;
			case Texture::TextureFormat::RGBA32:
				format = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
				break;
			}

			static_cast<impl::VulkanFramebuffer*>(framebuffer[core::ThreadType::Game].get())->CreateOffScreen(width, height, format, bReadUsage);
			static_cast<impl::VulkanFramebuffer*>(framebuffer[core::ThreadType::Render].get())->CreateOffScreen(width, height, format, bReadUsage);
		}
		buffer[core::ThreadType::Game] = std::make_unique<VulkanTextureBuffer>();
		buffer[core::ThreadType::Game]->Create(*framebuffer[core::ThreadType::Game].get());
		buffer[core::ThreadType::Render] = std::make_unique<VulkanTextureBuffer>();
		buffer[core::ThreadType::Render]->Create(*framebuffer[core::ThreadType::Render].get());
	}

	SH_RENDER_API auto RenderTexture::GetFramebuffer(core::ThreadType thr) const -> Framebuffer*
	{
		return framebuffer[thr].get();
	}
	SH_RENDER_API auto RenderTexture::GetPixelData() const -> const std::vector<Byte>&
	{
		assert(renderer->apiType == RenderAPI::Vulkan); // TODO
		if (renderer->apiType == RenderAPI::Vulkan)
		{

		}
		return pixels;
	}
	inline void RenderTexture::Resize(uint32_t width, uint32_t height)
	{
		if (this->renderer == nullptr)
			return;

		if (this->renderer->apiType == RenderAPI::Vulkan)
		{
			VkFormat format = VkFormat::VK_FORMAT_R8G8B8A8_SRGB;
			switch (this->format)
			{
			case Texture::TextureFormat::SRGB24:
				format = VkFormat::VK_FORMAT_R8G8B8_SRGB;
				break;
			case Texture::TextureFormat::RGB24:
				format = VkFormat::VK_FORMAT_R8G8B8_UNORM;
				break;
			case Texture::TextureFormat::RGBA32:
				format = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
				break;
			}

			static_cast<impl::VulkanFramebuffer*>(framebuffer[core::ThreadType::Game].get())->Clean();
			static_cast<impl::VulkanFramebuffer*>(framebuffer[core::ThreadType::Game].get())->CreateOffScreen(width, height, format, bReadUsage);
		}
		buffer[core::ThreadType::Game]->Clean();
		buffer[core::ThreadType::Game]->Create(*framebuffer[core::ThreadType::Game].get());
	}
	SH_RENDER_API void RenderTexture::SetSize(uint32_t width, uint32_t height)
	{
		this->width = width;
		this->height = height;

		Resize(width, height);

		bChangeSize = true;
		SetDirty();
	}
	SH_RENDER_API auto RenderTexture::GetSize() const -> glm::vec2
	{
		return { width, height };
	}

	SH_RENDER_API void RenderTexture::Sync()
	{
		Super::Sync();
		std::swap(framebuffer[core::ThreadType::Render], framebuffer[core::ThreadType::Game]);
		if (bChangeSize)
		{
			Resize(width, height);
			bChangeSize = false;
		}
	}
#if SH_EDITOR
	SH_RENDER_API void RenderTexture::OnPropertyChanged(const core::reflection::Property& property)
	{
		if (this->renderer == nullptr)
			return;
		if (property.GetName() == "width" || property.GetName() == "height")
		{
			Build(*renderer);
		}
	}
#endif
}