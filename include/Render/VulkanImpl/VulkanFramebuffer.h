#pragma once

#include "Export.h"
#include "VulkanImpl/VulkanConfig.h"

#include <vector>

namespace sh::render::impl
{
	class VulkanFramebuffer
	{
	private:
		VkDevice device;

		VkFramebuffer framebuffer;
		VkImageView img;
		VkRenderPass renderPass;

		uint32_t width, height;
	public:
		SH_RENDER_API VulkanFramebuffer(VkDevice device);
		SH_RENDER_API VulkanFramebuffer(const VulkanFramebuffer& other);
		SH_RENDER_API VulkanFramebuffer(VulkanFramebuffer&& other) noexcept;
		SH_RENDER_API ~VulkanFramebuffer();

		SH_RENDER_API auto operator=(const VulkanFramebuffer& other) -> VulkanFramebuffer&;
		SH_RENDER_API auto operator=(VulkanFramebuffer&& other) noexcept -> VulkanFramebuffer&;

		SH_RENDER_API auto Create(uint32_t width, uint32_t height, VkImageView img, VkRenderPass renderPass)->VkResult;
		SH_RENDER_API void Clean();

		SH_RENDER_API auto GetVkFramebuffer() const -> VkFramebuffer;
	};
}