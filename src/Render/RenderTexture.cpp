#include "RenderTexture.h"
#include "VulkanContext.h"
#include "VulkanTextureBuffer.h"
#include "VulkanFramebuffer.h"
#include "VulkanRenderPass.h"
#include "VulkanRenderPassManager.h"

namespace sh::render
{
	RenderTexture::RenderTexture(Texture::TextureFormat format, bool bMSAA) :
		Texture(format, 1024, 1024, false),

		width(1024), height(1024), 
		bMSAA(bMSAA)
	{
		SetAnisoLevel(0);
	}
	RenderTexture::~RenderTexture()
	{
	}

	RenderTexture::RenderTexture(RenderTexture&& other) noexcept :
		Texture(std::move(other)),

		width(other.width), height(other.height),
		framebuffer(std::move(other.framebuffer)),
		bMSAA(other.bMSAA)
	{

	}

	SH_RENDER_API void RenderTexture::SetReadUsage(bool bReadUsage) noexcept
	{
		this->bReadUsage = bReadUsage;
	}
	SH_RENDER_API void RenderTexture::Build(const IRenderContext& context)
	{
		if (framebuffer != nullptr)
			return;

		this->context = &context;
		CreateBuffers();
	}

	SH_RENDER_API auto RenderTexture::GetFramebuffer() const -> Framebuffer*
	{
		return framebuffer.get();
	}
	void RenderTexture::CreateBuffers()
	{
		if (context->GetRenderAPIType() == RenderAPI::Vulkan)
		{
			auto& vkContext = static_cast<const vk::VulkanContext&>(*context);

			VkFormat format = VkFormat::VK_FORMAT_R8G8B8A8_SRGB;
			switch (GetTextureFormat())
			{
			case Texture::TextureFormat::SRGB24:
				format = VkFormat::VK_FORMAT_R8G8B8_SRGB;
				break;
			case Texture::TextureFormat::R8:
				format = VkFormat::VK_FORMAT_R8_UNORM;
				break;
			case Texture::TextureFormat::RGB24:
				format = VkFormat::VK_FORMAT_R8G8B8_UNORM;
				break;
			case Texture::TextureFormat::RGBA32:
				format = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
				break;
			}

			vk::VulkanRenderPass::Config config;
			config.format = format;
			config.depthFormat = vkContext.FindSupportedDepthFormat(true);
			config.bOffScreen = true;
			config.bTransferSrc = bReadUsage;
			config.bUseStencil = true;
			config.sampleCount = bMSAA ? vkContext.GetSampleCount() : VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;

			auto& renderPass = vkContext.GetRenderPassManager().GetOrCreateRenderPass(config);

			if (framebuffer == nullptr)
				framebuffer = std::make_unique<vk::VulkanFramebuffer>(vkContext);
			static_cast<vk::VulkanFramebuffer*>(framebuffer.get())->CreateOffScreen(renderPass, width, height);
		}
		if (textureBuffer == nullptr)
			textureBuffer = std::make_unique<vk::VulkanTextureBuffer>();
		textureBuffer->Create(*framebuffer.get());
	}
	SH_RENDER_API void RenderTexture::SetSize(uint32_t width, uint32_t height)
	{
		assert(width != 0 && height != 0);
		this->width = width;
		this->height = height;

		bChangeSize = true;
		SyncDirty();
	}
	SH_RENDER_API auto RenderTexture::GetSize() const -> glm::vec2
	{
		return { width, height };
	}
	SH_RENDER_API auto RenderTexture::IsMSAA() const -> bool
	{
		return bMSAA;
	}

	SH_RENDER_API void RenderTexture::Sync()
	{
		Super::Sync();
		if (bChangeSize)
		{
			CreateBuffers();
			bChangeSize = false;
		}
	}
#if SH_EDITOR
	SH_RENDER_API void RenderTexture::OnPropertyChanged(const core::reflection::Property& property)
	{
		if (context == nullptr)
			return;
		if (property.GetName() == "width" || property.GetName() == "height")
		{
			Build(*context);
		}
	}
#endif
}