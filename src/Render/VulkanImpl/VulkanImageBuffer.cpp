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

	SH_RENDER_API auto VulkanImageBuffer::Create(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspectFlag, VkPhysicalDeviceProperties* gpuProp) -> VkResult
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

	SH_RENDER_API void VulkanImageBuffer::TransitionImageLayout(VulkanCommandBuffer* cmd, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = img;
		barrier.subresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.srcAccessMask = 0; // TODO
		barrier.dstAccessMask = 0; // TODO

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
		{
			barrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else 
		{
			throw std::invalid_argument("unsupported layout transition!");
		}

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		cmd->Build([&]()
		{
			vkCmdPipelineBarrier(
				cmd->GetCommandBuffer(),
				sourceStage, destinationStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);
		}, &beginInfo);

		context.GetQueueManager().SubmitCommand(*cmd);
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