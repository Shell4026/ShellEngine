#pragma once

#include "Export.h"
#include "VulkanConfig.h"

#include "VulkanImageBuffer.h"
#include "Render/Framebuffer.h"

#include "Core/NonCopyable.h"

#include <vector>
#include <memory>

namespace sh::render::vk
{
	class VulkanFramebuffer : public Framebuffer
	{
	private:
		VkDevice device;
		VkPhysicalDevice gpu;
		VmaAllocator alloc;

		VkFramebuffer framebuffer;
		VkImageView img;
		VkRenderPass renderPass;

		std::unique_ptr<VulkanImageBuffer> colorImg; //Only Offscreen
		std::unique_ptr<VulkanImageBuffer> depthImg;

		uint32_t width, height;
		VkFormat format;

		bool bTransferSrc = false;
	private:
		void CreateRenderPass();
		auto FindSupportedDepthFormat() -> VkFormat;
		void CreateDepthBuffer();
	public:
		SH_RENDER_API VulkanFramebuffer(VkDevice device, VkPhysicalDevice gpu, VmaAllocator alloc);
		SH_RENDER_API VulkanFramebuffer(VulkanFramebuffer&& other) noexcept;
		SH_RENDER_API ~VulkanFramebuffer();

		SH_RENDER_API auto operator=(VulkanFramebuffer&& other) noexcept -> VulkanFramebuffer&;

		SH_RENDER_API auto Create(uint32_t width, uint32_t height, VkImageView img, VkFormat format) -> VkResult;
		SH_RENDER_API auto CreateOffScreen(uint32_t width, uint32_t height, VkFormat format = VkFormat::VK_FORMAT_R8G8B8A8_SRGB, bool bTransferSrc = false) -> VkResult;
		SH_RENDER_API void Clean();
		SH_RENDER_API void TransferImageToBuffer(VulkanCommandBuffer* cmd, VkQueue queue, VkBuffer buffer, int x, int y);

		SH_RENDER_API auto GetRenderPass() const -> VkRenderPass;
		SH_RENDER_API auto GetVkFramebuffer() const -> VkFramebuffer;

		SH_RENDER_API auto GetColorImg() const -> VulkanImageBuffer*;
		SH_RENDER_API auto GetDepthImg() const -> VulkanImageBuffer*;

		SH_RENDER_API auto GetWidth() const -> uint32_t override;
		SH_RENDER_API auto GetHeight() const->uint32_t override;
	};
}