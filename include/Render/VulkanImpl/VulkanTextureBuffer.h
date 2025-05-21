#pragma once
#include "../Export.h"
#include "../ITextureBuffer.h"

#include "VulkanImageBuffer.h"
#include "VulkanCommandBuffer.h"

#include <memory>

namespace sh::render::vk
{
	class VulkanContext;
	class VulkanQueueManager;

	class VulkanTextureBuffer : public ITextureBuffer
	{
	private:
		const VulkanContext* context = nullptr;
		VulkanQueueManager* queueManager = nullptr;

		std::unique_ptr<VulkanImageBuffer> imgBuffer;
		std::unique_ptr<VulkanCommandBuffer> cmd;
		const Framebuffer* framebuffer;
		
		uint32_t width = 32, height = 32, channel = 4;
		
		VkFormat format = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;

		bool isRenderTexture;
	private:
		void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t mipLevel);
	public:
		SH_RENDER_API VulkanTextureBuffer();
		SH_RENDER_API VulkanTextureBuffer(VulkanTextureBuffer&& other) noexcept;
		SH_RENDER_API ~VulkanTextureBuffer();

		SH_RENDER_API void Create(const IRenderContext& context, const CreateInfo& info) override;
		SH_RENDER_API void Create(const Framebuffer& framebuffer) override;
		SH_RENDER_API void Clean() override;

		SH_RENDER_API void SetData(const void* data, uint32_t mipLevel = 1) override;

		SH_RENDER_API auto GetImageBuffer() const -> VulkanImageBuffer*;
		SH_RENDER_API auto GetSize() const -> std::size_t;
	};
}