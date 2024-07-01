#include "RenderTexture.h"

#include "Renderer.h"
#include "VulkanRenderer.h"
#include "VulkanTextureBuffer.h"
#include "VulkanImpl/VulkanFramebuffer.h"

namespace sh::render
{
	RenderTexture::RenderTexture() :
		Texture(TextureFormat::RGB24, 1024, 1024),

		renderer(nullptr),
		width(1024), height(1024)
	{
	}
	RenderTexture::~RenderTexture()
	{
	}

	RenderTexture::RenderTexture(RenderTexture&& other) noexcept :
		Texture(std::move(other)),
		renderer(other.renderer),

		width(other.width), height(other.height),
		framebuffer(std::move(other.framebuffer))
	{

	}

	void RenderTexture::Build(const Renderer& renderer)
	{
		this->renderer = &renderer;

		if (renderer.apiType == RenderAPI::Vulkan)
		{
			auto& vkRenderer = static_cast<const VulkanRenderer&>(renderer);
			framebuffer = std::make_unique<impl::VulkanFramebuffer>(vkRenderer.GetDevice(), vkRenderer.GetGPU(), vkRenderer.GetAllocator());
			static_cast<impl::VulkanFramebuffer*>(framebuffer.get())->CreateOffScreen(width, height);
		}
		buffer = std::make_unique<VulkanTextureBuffer>();
		buffer->Create(*framebuffer.get());
	}

	auto RenderTexture::GetFramebuffer() const -> Framebuffer*
	{
		return framebuffer.get();
	}

	void RenderTexture::SetSize(uint32_t width, uint32_t height)
	{
		this->width = width;
		this->height = height;

		if (this->renderer == nullptr)
			return;

		if (this->renderer->apiType == RenderAPI::Vulkan)
		{
			static_cast<impl::VulkanFramebuffer*>(framebuffer.get())->Clean();
			static_cast<impl::VulkanFramebuffer*>(framebuffer.get())->CreateOffScreen(width, height);
		}
		buffer->Clean();
		buffer->Create(*framebuffer.get());
	}
	auto RenderTexture::GetSize() const -> glm::vec2
	{
		return { width, height };
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