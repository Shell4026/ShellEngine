#pragma once

#include "VulkanConfig.h"
#include "Render/Export.h"

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

		VkImageView imgView;
		VkSampler sampler;

		bool bUseAnisotropy;
	public:
		SH_RENDER_API VulkanImageBuffer(VkDevice device, VkPhysicalDevice gpu, VmaAllocator allocator);
		SH_RENDER_API VulkanImageBuffer(VulkanImageBuffer&& other) noexcept;
		SH_RENDER_API ~VulkanImageBuffer();

		SH_RENDER_API void UseAnisotropy(bool bUse);

		SH_RENDER_API auto Create(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, 
			VkImageAspectFlags aspectFlag = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
			VkPhysicalDeviceProperties* gpuProp = nullptr) -> VkResult;
		SH_RENDER_API void Clean();

		SH_RENDER_API auto GetImage() const ->VkImage;
		SH_RENDER_API auto GetImageView() const -> VkImageView;
		SH_RENDER_API auto GetSampler() const -> VkSampler;
	};
}