#pragma once

#include "Export.h"
#include "VulkanConfig.h"

#include "VulkanImageBuffer.h"
#include "Render/Framebuffer.h"

#include <vector>

namespace sh::render::impl
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

		VulkanImageBuffer depthImg;

		uint32_t width, height;
		VkFormat format;
	private:
		void CreateRenderPass();
		auto FindSupportedDepthFormat() -> VkFormat;
	public:
		SH_RENDER_API VulkanFramebuffer(VkDevice device, VkPhysicalDevice gpu, VmaAllocator alloc);
		SH_RENDER_API VulkanFramebuffer(const VulkanFramebuffer& other);
		SH_RENDER_API VulkanFramebuffer(VulkanFramebuffer&& other) noexcept;
		SH_RENDER_API ~VulkanFramebuffer();

		SH_RENDER_API auto operator=(const VulkanFramebuffer& other) -> VulkanFramebuffer&;
		SH_RENDER_API auto operator=(VulkanFramebuffer&& other) noexcept -> VulkanFramebuffer&;

		SH_RENDER_API auto Create(uint32_t width, uint32_t height, VkImageView img, VkFormat format)->VkResult;
		SH_RENDER_API auto CreateDepthBuffer();
		SH_RENDER_API void Clean();

		SH_RENDER_API auto GetRenderPass() const -> VkRenderPass;
		SH_RENDER_API auto GetVkFramebuffer() const -> VkFramebuffer;
	};
}