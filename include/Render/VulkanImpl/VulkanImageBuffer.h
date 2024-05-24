#pragma once

#include "VulkanConfig.h"
#include "Render/Export.h"

#include "../vma-src/include/vk_mem_alloc.h"

#include <cstdint>

namespace sh::render::impl
{
	class VulkanImageBuffer
	{
	private:
		VkDevice device;
		VkPhysicalDevice gpu;
		VmaAllocator allocator;

		VkImage img;
		VmaAllocation imgMem;
	public:
		SH_RENDER_API VulkanImageBuffer(VkDevice device, VkPhysicalDevice gpu, VmaAllocator allocator);
		SH_RENDER_API VulkanImageBuffer(VulkanImageBuffer&& other) noexcept;
		SH_RENDER_API ~VulkanImageBuffer();

		SH_RENDER_API auto Create(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage) -> VkResult;
		SH_RENDER_API void Clean();

		SH_RENDER_API auto GetNative() const -> VkImage;
	};
}