#pragma once

#include "Export.h"
#include "ITextureBuffer.h"

#include "VulkanImpl/VulkanImageBuffer.h"
#include "VulkanImpl/VulkanCommandBuffer.h"

#include <memory>
namespace sh::render
{
	class VulkanRenderer;

	class VulkanTextureBuffer : public ITextureBuffer
	{
	private:
		std::unique_ptr<impl::VulkanImageBuffer> buffer;

		std::unique_ptr<impl::VulkanCommandBuffer> cmd;
	private:
		void TransitionImageLayout(VkQueue queue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyBufferToImage(VkQueue queue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	public:
		SH_RENDER_API VulkanTextureBuffer();
		SH_RENDER_API VulkanTextureBuffer(VulkanTextureBuffer&& other) noexcept;
		SH_RENDER_API ~VulkanTextureBuffer() = default;

		SH_RENDER_API void Create(const VulkanRenderer& renderer, const void* data, uint32_t width, uint32_t height, Texture::TextureFormat format) override;
		SH_RENDER_API void Bind() override;
	};
}