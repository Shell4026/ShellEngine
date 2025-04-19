#include "VulkanImageBuffer.h"
#include "VulkanContext.h"
#include "VulkanQueueManager.h"

#include <cassert>

namespace sh::render::vk
{
	SH_RENDER_API VulkanImageBuffer::VulkanImageBuffer(const VulkanContext& context) :
		context(context),
		device(context.GetDevice()), gpu(context.GetGPU()), allocator(context.GetAllocator()),
		img(nullptr), imgMem(nullptr), imgView(nullptr), sampler(nullptr),
		bUseAnisotropy(false)
	{

	}
	SH_RENDER_API VulkanImageBuffer::VulkanImageBuffer(VulkanImageBuffer&& other) noexcept :
		context(other.context),
		device(other.device), gpu(other.gpu), allocator(other.allocator),
		img(other.img), imgMem(other.imgMem), imgView(other.imgView), sampler(other.sampler),
		mipCount(other.mipCount),
		bUseAnisotropy(other.bUseAnisotropy)
	{
		other.img = nullptr;
		other.imgMem = nullptr;
		other.imgView = nullptr;
		other.sampler = nullptr;
	}
	SH_RENDER_API auto VulkanImageBuffer::operator=(VulkanImageBuffer&& other) noexcept -> VulkanImageBuffer&
	{
		device = other.device;
		gpu = other.gpu;
		allocator = other.allocator;

		img = other.img;
		imgMem = other.imgMem;
		imgView = other.imgView;
		sampler = other.sampler;

		other.img = nullptr;
		other.imgMem = nullptr;
		other.imgView = nullptr;
		other.sampler = nullptr;

		mipCount = other.mipCount;
		bUseAnisotropy = other.bUseAnisotropy;

		return *this;
	}

	SH_RENDER_API VulkanImageBuffer::~VulkanImageBuffer()
	{
		Clean();
	}
	SH_RENDER_API void VulkanImageBuffer::Clean()
	{
		if (sampler)
		{
			vkDestroySampler(device, sampler, nullptr);
			sampler = nullptr;
		}
		if (imgView)
		{
			vkDestroyImageView(device, imgView, nullptr);
			imgView = nullptr;
		}
		if (img)
		{
			vmaDestroyImage(allocator, img, imgMem);
			img = nullptr;
			imgMem = nullptr;
		}
	}

	SH_RENDER_API void VulkanImageBuffer::UseAnisotropy(bool bUse)
	{
		bUseAnisotropy = bUse;
	}

	SH_RENDER_API auto VulkanImageBuffer::Create(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspectFlag, VkSampleCountFlagBits sampleCount) -> VkResult
	{
		Clean();

		VkImageCreateInfo info{};
		info.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.imageType = VkImageType::VK_IMAGE_TYPE_2D;
		info.extent.width = width;
		info.extent.height = height;
		info.extent.depth = 1;
		info.mipLevels = (sampleCount == VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT) ? mipCount : 1;
		info.arrayLayers = 1;
		info.format = format;
		info.tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
		info.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
		info.usage = usage;
		info.sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
		info.samples = sampleCount;
		info.flags = 0;

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO;

		auto result = vmaCreateImage(allocator, &info, &allocCreateInfo, &img, &imgMem, nullptr);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			return result;

		VkImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.image = img;
		viewCreateInfo.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.format = format;
		viewCreateInfo.subresourceRange.aspectMask = aspectFlag;
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.levelCount = 1;
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = 1;

		result = vkCreateImageView(device, &viewCreateInfo, nullptr, &imgView);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			return result;

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VkFilter::VK_FILTER_LINEAR;
		samplerInfo.minFilter = VkFilter::VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = bUseAnisotropy;
		samplerInfo.maxAnisotropy = bUseAnisotropy ? context.GetGPUProperty().limits.maxSamplerAnisotropy : 0;
		samplerInfo.borderColor = VkBorderColor::VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = false;
		samplerInfo.compareEnable = false;
		samplerInfo.compareOp = VkCompareOp::VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		result = vkCreateSampler(device, &samplerInfo, nullptr, &sampler);
		assert(result == VkResult::VK_SUCCESS);
		return result;
	}

	SH_RENDER_API auto VulkanImageBuffer::GetImage() const -> VkImage
	{
		return img;
	}
	SH_RENDER_API auto VulkanImageBuffer::GetImageView() const -> VkImageView
	{
		return imgView;
	}
	SH_RENDER_API auto VulkanImageBuffer::GetSampler() const -> VkSampler
	{
		return sampler;
	}
}