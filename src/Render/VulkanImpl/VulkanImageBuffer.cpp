#include "VulkanImageBuffer.h"

#include "VulkanBuffer.h"

#include <cassert>

namespace sh::render::impl
{
	VulkanImageBuffer::VulkanImageBuffer(VkDevice device, VkPhysicalDevice gpu, VmaAllocator allocator) :
		device(device), gpu(gpu), allocator(allocator),
		img(nullptr), imgMem(nullptr)
	{

	}
	VulkanImageBuffer::VulkanImageBuffer(VulkanImageBuffer&& other) noexcept :
		device(other.device), gpu(other.gpu), allocator(other.allocator),
		img(other.img), imgMem(other.imgMem)
	{
		other.img = nullptr;
		other.imgMem = nullptr;
	}

	VulkanImageBuffer::~VulkanImageBuffer()
	{
		Clean();
	}
	void VulkanImageBuffer::Clean()
	{
		if (img)
		{
			vmaDestroyImage(allocator, img, imgMem);
			img = nullptr;
			imgMem = nullptr;
		}
	}

	auto VulkanImageBuffer::Create(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage) -> VkResult
	{
		VkImageCreateInfo info{};
		info.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.imageType = VkImageType::VK_IMAGE_TYPE_2D;
		info.extent.width = width;
		info.extent.height = height;
		info.extent.depth = 1;
		info.mipLevels = 1;
		info.arrayLayers = 1;
		info.format = format;
		info.tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
		info.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
		info.usage = usage;
		info.sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
		info.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
		info.flags = 0;

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO;

		auto result = vmaCreateImage(allocator, &info, &allocCreateInfo, &img, &imgMem, nullptr);
		assert(result == VkResult::VK_SUCCESS);
		return result;
	}

	auto VulkanImageBuffer::GetNative() const -> VkImage
	{
		return img;
	}
}