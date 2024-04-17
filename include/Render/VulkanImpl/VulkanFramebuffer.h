#pragma once

#include "Export.h"
#include "VulkanImpl/VulkanConfig.h"

#include <vector>

namespace sh::render::impl
{
	class VulkanPipeline;

	class VulkanFramebuffer
	{
	private:
		const VulkanPipeline& pipeline;

		VkFramebuffer framebuffer;
		VkImageView img;

		uint32_t width, height;
	public:
		SH_RENDER_API VulkanFramebuffer(const VulkanPipeline& pipeline);
		SH_RENDER_API VulkanFramebuffer(const VulkanFramebuffer& other);
		SH_RENDER_API VulkanFramebuffer(VulkanFramebuffer&& other) noexcept;
		SH_RENDER_API ~VulkanFramebuffer();

		SH_RENDER_API auto operator=(const VulkanFramebuffer& other) -> VulkanFramebuffer&;
		SH_RENDER_API auto operator=(VulkanFramebuffer&& other) noexcept -> VulkanFramebuffer&;

		SH_RENDER_API auto Create(uint32_t width, uint32_t height, VkImageView img)->VkResult;
		SH_RENDER_API void Clean();

		SH_RENDER_API auto GetVkFramebuffer() -> VkFramebuffer;
	};
}