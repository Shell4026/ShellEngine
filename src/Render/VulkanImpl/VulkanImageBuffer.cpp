﻿#include "VulkanImageBuffer.h"

#include "VulkanBuffer.h"

#include <cassert>

namespace sh::render::impl
{
	VulkanImageBuffer::VulkanImageBuffer(VkDevice device, VkPhysicalDevice gpu, VmaAllocator allocator) :
		device(device), gpu(gpu), allocator(allocator),
		img(nullptr), imgMem(nullptr), imgView(nullptr), sampler(nullptr),
		bUseAnisotropy(false)
	{

	}
	VulkanImageBuffer::VulkanImageBuffer(VulkanImageBuffer&& other) noexcept :
		device(other.device), gpu(other.gpu), allocator(other.allocator),
		img(other.img), imgMem(other.imgMem), imgView(other.imgView), sampler(other.sampler),
		bUseAnisotropy(other.bUseAnisotropy)
	{
		other.img = nullptr;
		other.imgMem = nullptr;
		other.imgView = nullptr;
		other.sampler = nullptr;
	}

	VulkanImageBuffer::~VulkanImageBuffer()
	{
		Clean();
	}
	void VulkanImageBuffer::Clean()
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

	void VulkanImageBuffer::UseAnisotropy(bool bUse)
	{
		bUseAnisotropy = bUse;
	}

	auto VulkanImageBuffer::Create(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkPhysicalDeviceProperties* gpuProp) -> VkResult
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
		if (result != VkResult::VK_SUCCESS)
			return result;

		VkImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.image = img;
		viewCreateInfo.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.format = format;
		viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
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
		samplerInfo.maxAnisotropy = 0;
		if (bUseAnisotropy)
		{
			if(gpuProp)
				samplerInfo.maxAnisotropy = gpuProp->limits.maxSamplerAnisotropy;
			else
			{
				VkPhysicalDeviceProperties properties{};
				vkGetPhysicalDeviceProperties(gpu, &properties);
				samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
			}
		}
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

	auto VulkanImageBuffer::GetImage() const -> VkImage
	{
		return img;
	}
	auto VulkanImageBuffer::GetImageView() const -> VkImageView
	{
		return imgView;
	}
	auto VulkanImageBuffer::GetSampler() const -> VkSampler
	{
		return sampler;
	}
}