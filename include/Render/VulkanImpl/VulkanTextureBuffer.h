#pragma once

#include "../Export.h"
#include "../ITextureBuffer.h"

#include "VulkanImageBuffer.h"
#include "VulkanCommandBuffer.h"

#include <memory>
namespace sh::render::vk
{
	class VulkanRenderer;

	class VulkanTextureBuffer : public ITextureBuffer
	{
	private:
		std::unique_ptr<VulkanImageBuffer> buffer;

		std::unique_ptr<VulkanCommandBuffer> cmd;

		const Framebuffer* framebuffer;

		bool isRenderTexture;
	private:
		void TransitionImageLayout(VkQueue queue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyBufferToImage(VkQueue queue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	public:
		SH_RENDER_API VulkanTextureBuffer();
		SH_RENDER_API VulkanTextureBuffer(VulkanTextureBuffer&& other) noexcept;
		SH_RENDER_API ~VulkanTextureBuffer();

		SH_RENDER_API void Create(const Renderer& renderer, const void* data, uint32_t width, uint32_t height, Texture::TextureFormat format) override;
		SH_RENDER_API void Create(const Framebuffer& framebuffer) override;
		SH_RENDER_API void Clean() override;

		SH_RENDER_API auto GetImageBuffer() const -> VulkanImageBuffer*;
	};
}