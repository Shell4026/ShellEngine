#include "VulkanImageBuffer.h"
#include "VulkanContext.h"
#include "VulkanQueueManager.h"

#include <cassert>

namespace sh::render::vk
{
	SH_RENDER_API VulkanImageBuffer::VulkanImageBuffer(const VulkanContext& context) :
		context(context),
		device(context.GetDevice()), gpu(context.GetGPU()), allocator(context.GetAllocator()),
		img(nullptr), imgMem(nullptr), imgView(nullptr), sampler(nullptr)
	{

	}
	SH_RENDER_API VulkanImageBuffer::VulkanImageBuffer(VulkanImageBuffer&& other) noexcept :
		context(other.context),
		device(other.device), gpu(other.gpu), allocator(other.allocator),
		img(other.img), imgMem(other.imgMem), imgView(other.imgView), sampler(other.sampler),
		layout(other.layout),
		mipLevels(other.mipLevels),
		aniso(other.aniso)
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

		layout = other.layout;

		mipLevels = other.mipLevels;
		aniso = other.aniso;

		return *this;
	}

	SH_RENDER_API VulkanImageBuffer::~VulkanImageBuffer()
	{
		Clean();
	}
	SH_RENDER_API void VulkanImageBuffer::Clean()
	{
		layout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
		if (sampler != nullptr)
		{
			vkDestroySampler(device, sampler, nullptr);
			sampler = nullptr;
		}
		if (imgView != nullptr)
		{
			vkDestroyImageView(device, imgView, nullptr);
			imgView = nullptr;
		}
		if (img != nullptr && !bOtherImg)
		{
			vmaDestroyImage(allocator, img, imgMem);
			img = nullptr;
			imgMem = nullptr;
		}
		bOtherImg = false;
	}
	SH_RENDER_API auto VulkanImageBuffer::SetFilter(VkFilter filter) -> VulkanImageBuffer&
	{
		this->filter = filter;
		return *this;
	}
	SH_RENDER_API auto VulkanImageBuffer::SetAnisotropy(uint32_t aniso) -> VulkanImageBuffer&
	{
		this->aniso = aniso;
		return *this;
	}

	SH_RENDER_API auto VulkanImageBuffer::Create(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImageAspectFlags aspectFlag, VkSampleCountFlagBits sampleCount, uint32_t mipLevels) -> VkResult
	{
		Clean();
		this->mipLevels = mipLevels;

		VkImageCreateInfo info{};
		info.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.imageType = VkImageType::VK_IMAGE_TYPE_2D;
		info.extent.width = width;
		info.extent.height = height;
		info.extent.depth = 1;
		info.mipLevels = (sampleCount == VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT) ? mipLevels : 1;
		info.arrayLayers = 1;
		info.format = format;
		info.tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
		info.initialLayout = layout;
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
		viewCreateInfo.subresourceRange.levelCount = mipLevels;
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = 1;

		result = vkCreateImageView(device, &viewCreateInfo, nullptr, &imgView);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			return result;

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = filter;
		samplerInfo.minFilter = filter;
		samplerInfo.addressModeU = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = aniso > 0;
		samplerInfo.maxAnisotropy = static_cast<float>(aniso);
		samplerInfo.borderColor = VkBorderColor::VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = false;
		samplerInfo.compareEnable = false;
		samplerInfo.compareOp = VkCompareOp::VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = static_cast<float>(mipLevels);

		result = vkCreateSampler(device, &samplerInfo, nullptr, &sampler);
		assert(result == VkResult::VK_SUCCESS);
		return result;
	}
	SH_RENDER_API auto VulkanImageBuffer::Create(VkImage image, VkFormat format) -> VkResult
	{
		img = image;

		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = img;
		createInfo.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = format;
		createInfo.components.r = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		auto result = vkCreateImageView(context.GetDevice(), &createInfo, nullptr, &imgView);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			throw std::runtime_error(std::string{ "vkCreateImageView()" } + string_VkResult(result));

		bOtherImg = true;

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

	SH_RENDER_API void VulkanImageBuffer::LayoutChangedByRenderPass(VkImageLayout layout)
	{
		this->layout = layout;
	}
	SH_RENDER_API auto VulkanImageBuffer::GetLayout() const -> VkImageLayout
	{
		return layout;
	}
	SH_RENDER_API void VulkanImageBuffer::ChangeLayoutCommand(VkCommandBuffer cmd, VkImageLayout newLayout)
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = layout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = img;
		barrier.subresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = 0;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (layout == VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (layout == VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

			sourceStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		else if (layout == VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = 0;

			sourceStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		}
		else if (layout == VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;
			barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			destinationStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (layout == VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (layout == VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR && newLayout == VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_MEMORY_READ_BIT;
			barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

			sourceStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			destinationStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		else if (layout == VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

			barrier.subresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT | VkImageAspectFlagBits::VK_IMAGE_ASPECT_STENCIL_BIT;
		}
		else
		{
			throw std::invalid_argument("Unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			cmd,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		layout = newLayout;
	}
}