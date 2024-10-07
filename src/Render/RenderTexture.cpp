#include "pch.h"
#include "RenderTexture.h"

#include "Renderer.h"
#include "VulkanRenderer.h"
#include "VulkanTextureBuffer.h"
#include "VulkanImpl/VulkanFramebuffer.h"

namespace sh::render
{
	RenderTexture::RenderTexture() :
		Texture(TextureFormat::RGB24, 1024, 1024),

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

	void RenderTexture::Build(Renderer& renderer)
	{
		this->renderer = &renderer;

		if (renderer.apiType == RenderAPI::Vulkan)
		{
			auto& vkRenderer = static_cast<const VulkanRenderer&>(renderer);
			framebuffer[core::ThreadType::Game] = std::make_unique<impl::VulkanFramebuffer>(vkRenderer.GetDevice(), vkRenderer.GetGPU(), vkRenderer.GetAllocator());
			framebuffer[core::ThreadType::Render] = std::make_unique<impl::VulkanFramebuffer>(vkRenderer.GetDevice(), vkRenderer.GetGPU(), vkRenderer.GetAllocator());
			static_cast<impl::VulkanFramebuffer*>(framebuffer[core::ThreadType::Game].get())->CreateOffScreen(width, height);
			static_cast<impl::VulkanFramebuffer*>(framebuffer[core::ThreadType::Render].get())->CreateOffScreen(width, height);
		}
		buffer[core::ThreadType::Game] = std::make_unique<VulkanTextureBuffer>();
		buffer[core::ThreadType::Game]->Create(*framebuffer[core::ThreadType::Game].get());
		buffer[core::ThreadType::Render] = std::make_unique<VulkanTextureBuffer>();
		buffer[core::ThreadType::Render]->Create(*framebuffer[core::ThreadType::Render].get());

		SetDirty();
	}

	auto RenderTexture::GetFramebuffer() const -> Framebuffer*
	{
		return framebuffer[core::ThreadType::Render].get();
	}

	void RenderTexture::SetSize(uint32_t width, uint32_t height)
	{
		this->width = width;
		this->height = height;

		if (this->renderer == nullptr)
			return;

		if (this->renderer->apiType == RenderAPI::Vulkan)
		{
			static_cast<impl::VulkanFramebuffer*>(framebuffer[core::ThreadType::Game].get())->Clean();
			static_cast<impl::VulkanFramebuffer*>(framebuffer[core::ThreadType::Game].get())->CreateOffScreen(width, height);
		}
		buffer[core::ThreadType::Game]->Clean();
		buffer[core::ThreadType::Game]->Create(*framebuffer[core::ThreadType::Game].get());

		SetDirty();
	}
	auto RenderTexture::GetSize() const -> glm::vec2
	{
		return { width, height };
	}

	void RenderTexture::Sync()
	{
		Super::Sync();
		std::swap(framebuffer[core::ThreadType::Render], framebuffer[core::ThreadType::Game]);
	}
#if SH_EDITOR
	void RenderTexture::OnPropertyChanged(const core::reflection::Property& property)
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