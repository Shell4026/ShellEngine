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
			framebuffer[GAME_THREAD] = std::make_unique<impl::VulkanFramebuffer>(vkRenderer.GetDevice(), vkRenderer.GetGPU(), vkRenderer.GetAllocator());
			framebuffer[RENDER_THREAD] = std::make_unique<impl::VulkanFramebuffer>(vkRenderer.GetDevice(), vkRenderer.GetGPU(), vkRenderer.GetAllocator());
			static_cast<impl::VulkanFramebuffer*>(framebuffer[GAME_THREAD].get())->CreateOffScreen(width, height);
			static_cast<impl::VulkanFramebuffer*>(framebuffer[RENDER_THREAD].get())->CreateOffScreen(width, height);
		}
		buffer[GAME_THREAD] = std::make_unique<VulkanTextureBuffer>();
		buffer[GAME_THREAD]->Create(*framebuffer[GAME_THREAD].get());
		buffer[RENDER_THREAD] = std::make_unique<VulkanTextureBuffer>();
		buffer[RENDER_THREAD]->Create(*framebuffer[RENDER_THREAD].get());

		SetDirty();
	}

	auto RenderTexture::GetFramebuffer() const -> Framebuffer*
	{
		return framebuffer[RENDER_THREAD].get();
	}

	void RenderTexture::SetSize(uint32_t width, uint32_t height)
	{
		this->width = width;
		this->height = height;

		if (this->renderer == nullptr)
			return;

		if (this->renderer->apiType == RenderAPI::Vulkan)
		{
			static_cast<impl::VulkanFramebuffer*>(framebuffer[GAME_THREAD].get())->Clean();
			static_cast<impl::VulkanFramebuffer*>(framebuffer[GAME_THREAD].get())->CreateOffScreen(width, height);
		}
		buffer[GAME_THREAD]->Clean();
		buffer[GAME_THREAD]->Create(*framebuffer[GAME_THREAD].get());

		SetDirty();
	}
	auto RenderTexture::GetSize() const -> glm::vec2
	{
		return { width, height };
	}

	void RenderTexture::Sync()
	{
		Super::Sync();
		std::swap(framebuffer[RENDER_THREAD], framebuffer[GAME_THREAD]);
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