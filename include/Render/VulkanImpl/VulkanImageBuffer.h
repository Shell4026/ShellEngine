#pragma once
#include "VulkanCommandBuffer.h"
#include "Render/Export.h"

#include "Core/NonCopyable.h"

#include <cstdint>

namespace sh::render::vk
{
	class VulkanContext;

	class VulkanImageBuffer : public core::INonCopyable
	{
	public:
		SH_RENDER_API VulkanImageBuffer(const VulkanContext& context);
		SH_RENDER_API VulkanImageBuffer(VulkanImageBuffer&& other) noexcept;
		SH_RENDER_API ~VulkanImageBuffer();

		SH_RENDER_API auto operator=(VulkanImageBuffer&& other) noexcept -> VulkanImageBuffer&;

		SH_RENDER_API auto SetAnisotropy(uint32_t aniso) -> VulkanImageBuffer&;
		SH_RENDER_API auto SetFilter(VkFilter filter) -> VulkanImageBuffer&;

		SH_RENDER_API auto Create(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, 
			VkImageAspectFlags aspectFlag = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT, 
			VkSampleCountFlagBits sampleCount = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT, uint32_t mipLevels = 1) -> VkResult;
		SH_RENDER_API auto Create(VkImage image, VkFormat format) -> VkResult;
		SH_RENDER_API void Clean();

		SH_RENDER_API auto GetImage() const -> VkImage;
		SH_RENDER_API auto GetImageView() const -> VkImageView;
		SH_RENDER_API auto GetSampler() const -> VkSampler;

		/// @brief 렌더 패스에 의해 레이아웃이 변경 됐다면 호출
		/// @param layout 이미지 레이아웃
		SH_RENDER_API void LayoutChangedByRenderPass(VkImageLayout layout);
		SH_RENDER_API auto GetLayout() const -> VkImageLayout;

		SH_RENDER_API void ChangeLayoutCommand(VkCommandBuffer cmd, VkImageLayout newLayout);
	private:
		const VulkanContext& context;

		VkDevice device;
		VkPhysicalDevice gpu;
		VmaAllocator allocator;

		VkImage img;
		VmaAllocation imgMem;

		VkImageView imgView;
		VkSampler sampler;

		VkImageLayout layout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;

		uint32_t aniso = 0;
		uint32_t mipLevels = 1;

		VkFilter filter = VkFilter::VK_FILTER_LINEAR;

		bool bOtherImg = false;
	};
}