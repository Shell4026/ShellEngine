#include "VulkanImageBuffer.h"
#include "VulkanContext.h"
#include "VulkanQueueManager.h"
#include "VulkanSwapChain.h"
#include "VulkanBuffer.h"
#include "VulkanCommandBufferPool.h"
#include "VulkanCommandBuffer.h"

#include <cassert>

namespace sh::render::vk
{
	SH_RENDER_API VulkanImageBuffer::VulkanImageBuffer()
	{
	}
	SH_RENDER_API VulkanImageBuffer::VulkanImageBuffer(VulkanImageBuffer&& other) noexcept :
		ctx(other.ctx),
		img(other.img), imgMem(other.imgMem), imgView(other.imgView), sampler(other.sampler),
		
		aspect(other.aspect),
		filter(other.filter),
		format(other.format),
		sample(other.sample),
		width(other.width),
		height(other.height),
		channel(other.channel),
		aniso(other.aniso),
		mipLevel(other.mipLevel),

		bSwapChainImg(other.bSwapChainImg),
		bRenderTarget(other.bRenderTarget)
	{
		other.Clear();
	}
	SH_RENDER_API auto VulkanImageBuffer::operator=(VulkanImageBuffer&& other) noexcept -> VulkanImageBuffer&
	{
		Clear();

		ctx = other.ctx;

		img = other.img;
		imgMem = other.imgMem;
		imgView = other.imgView;
		sampler = other.sampler;

		aspect = other.aspect;
		filter = other.filter;
		format = other.format;
		sample = other.sample;
		width = other.width;
		height = other.height;
		channel = other.channel;
		aniso = other.aniso;
		mipLevel = other.mipLevel;

		bSwapChainImg = other.bSwapChainImg;
		bRenderTarget = other.bRenderTarget;

		other.img = VK_NULL_HANDLE;
		other.imgMem = VK_NULL_HANDLE;
		other.imgView = VK_NULL_HANDLE;
		other.sampler = VK_NULL_HANDLE;
		other.bSwapChainImg = false;
		other.bRenderTarget = false;

		return *this;
	}

	SH_RENDER_API VulkanImageBuffer::~VulkanImageBuffer()
	{
		Clear();
	}
	SH_RENDER_API auto VulkanImageBuffer::Create(const IRenderContext& context, const CreateInfo& info) -> bool
	{
		Clear();
		ctx = &static_cast<const VulkanContext&>(context);

		const uint32_t actualMip = info.bMSAAImg ? 1 : info.mipLevel;
		assert(actualMip > 0);

		const bool bColorImg = !IsDepthTexture(info.format);
		format = ConvertTextureFormat(info.format);
		sample = info.bMSAAImg ? ctx->GetSampleCount() : VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
		aspect = bColorImg ? 
			(VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT) : 
			(VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT | VkImageAspectFlagBits::VK_IMAGE_ASPECT_STENCIL_BIT);
		filter = info.filtering == 0 ? VkFilter::VK_FILTER_NEAREST : VkFilter::VK_FILTER_LINEAR;
		
		width = info.width == 0 ? 1 : info.width;
		height = info.height == 0 ? 1 : info.height;
		channel = GetChannelCount(format);
		aniso = info.aniso;
		mipLevel = actualMip;

		bRenderTarget = info.bRenderTarget;

		VkImageUsageFlags usage;
		if (info.bMSAAImg)
		{
			if (bColorImg)
				usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			else
				usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		}
		else
		{
			if (bColorImg)
			{
				if (info.bRenderTarget)
					usage = 
					VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | 
					VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT | 
					VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
				else
					usage = 
					VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT | 
					VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT;
			}
			else
				usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		}
		VkImageCreateInfo imageCi{};
		imageCi.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCi.imageType = VkImageType::VK_IMAGE_TYPE_2D;
		imageCi.extent.width = width;
		imageCi.extent.height = height;
		imageCi.extent.depth = 1;
		imageCi.mipLevels = actualMip;
		imageCi.arrayLayers = 1;
		imageCi.format = format;
		imageCi.tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
		imageCi.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
		imageCi.usage = usage;
		imageCi.sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
		imageCi.samples = sample;
		imageCi.flags = 0;

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO;

		auto result = vmaCreateImage(ctx->GetAllocator(), &imageCi, &allocCreateInfo, &img, &imgMem, nullptr);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			return false;

		VkImageViewCreateInfo viewCreateInfo{};
		viewCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCreateInfo.image = img;
		viewCreateInfo.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
		viewCreateInfo.format = format;
		viewCreateInfo.subresourceRange.aspectMask = aspect;
		viewCreateInfo.subresourceRange.baseMipLevel = 0;
		viewCreateInfo.subresourceRange.levelCount = actualMip;
		viewCreateInfo.subresourceRange.baseArrayLayer = 0;
		viewCreateInfo.subresourceRange.layerCount = 1;

		result = vkCreateImageView(ctx->GetDevice(), &viewCreateInfo, nullptr, &imgView);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
		{
			Clear();
			return false;
		}

		if (!info.bMSAAImg)
		{
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
			samplerInfo.maxLod = static_cast<float>(mipLevel - 1);

			result = vkCreateSampler(ctx->GetDevice(), &samplerInfo, nullptr, &sampler);
			assert(result == VkResult::VK_SUCCESS);
			if (result != VkResult::VK_SUCCESS)
			{
				Clear();
				return false;
			}
		}
		return true;
	}
	SH_RENDER_API void VulkanImageBuffer::Clear()
	{
		aspect = VkImageAspectFlagBits::VK_IMAGE_ASPECT_NONE;
		filter = VkFilter::VK_FILTER_LINEAR;
		format = VkFormat::VK_FORMAT_UNDEFINED;
		width = height = 0;
		channel = 4;
		aniso = 0;
		mipLevel = 1;

		if (ctx != nullptr)
		{
			if (sampler != VK_NULL_HANDLE)
			{
				vkDestroySampler(ctx->GetDevice(), sampler, nullptr);
				sampler = VK_NULL_HANDLE;
			}
			if (imgView != VK_NULL_HANDLE)
			{
				vkDestroyImageView(ctx->GetDevice(), imgView, nullptr);
				imgView = VK_NULL_HANDLE;
			}
			if (img != VK_NULL_HANDLE && !bSwapChainImg)
			{
				vmaDestroyImage(ctx->GetAllocator(), img, imgMem);
				img = VK_NULL_HANDLE;
				imgMem = VK_NULL_HANDLE;
			}
		}
		bSwapChainImg = false;
		bRenderTarget = false;
	}

	SH_RENDER_API void VulkanImageBuffer::SetData(const void* data, uint32_t mipLevel)
	{
		assert(!bRenderTarget);
		if (ctx == nullptr || img == VK_NULL_HANDLE || bRenderTarget)
			return;

		VulkanBuffer stagingBuffer{ *ctx };
		
		uint32_t mipWidth = width;
		uint32_t mipHeight = height;
		for (int i = 0; i < mipLevel; ++i)
		{
			mipWidth = std::max(1u, mipWidth / 2);
			mipHeight = std::max(1u, mipHeight / 2);
		}
		std::size_t bufferSize = mipWidth * mipHeight * channel;
		
		stagingBuffer.Create(bufferSize,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		stagingBuffer.SetData(data);

		VulkanCommandBuffer* cmd = ctx->GetCommandBufferPool().AllocateCommandBuffer(std::this_thread::get_id(), VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT);
		cmd->Build([&]
			{
				BarrierCommand(cmd->GetCommandBuffer(), *this, 
					VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
					VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
					VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT,
					VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT
				);
				VkBufferImageCopy region{};
				region.bufferOffset = 0;
				region.bufferRowLength = 0;
				region.bufferImageHeight = 0;

				region.imageSubresource.aspectMask = aspect;
				region.imageSubresource.mipLevel = mipLevel;
				region.imageSubresource.baseArrayLayer = 0;
				region.imageSubresource.layerCount = 1;

				region.imageOffset = { 0, 0, 0 };
				region.imageExtent =
				{
					mipWidth,
					mipHeight,
					1
				};

				vkCmdCopyBufferToImage(cmd->GetCommandBuffer(), stagingBuffer.GetBuffer(), img, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

				BarrierCommand(cmd->GetCommandBuffer(), *this,
					VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
					VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT,
					VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT
				);
			},
			true
		);
		
		VkFence fence = cmd->GetOrCreateFence();
		ctx->GetQueueManager().Submit(VulkanQueueManager::Role::Graphics, *cmd, fence);
		vkWaitForFences(ctx->GetDevice(), 1, &fence, true, std::numeric_limits<uint64_t>::max());
		vkResetFences(ctx->GetDevice(), 1, &fence);
		
		ctx->GetCommandBufferPool().DeallocateCommandBuffer(*cmd);
	}

	SH_RENDER_API void VulkanImageBuffer::CreateFromSwapChain(const IRenderContext& context, VkImage img)
	{
		Clear();
		ctx = &static_cast<const VulkanContext&>(context);

		VulkanSwapChain& swapChain = ctx->GetSwapChain();

		this->img = img;
		imgMem = VK_NULL_HANDLE;
		sampler = VK_NULL_HANDLE;

		aspect = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
		filter = VkFilter::VK_FILTER_LINEAR;
		format = swapChain.GetSwapChainImageFormat();
		sample = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
		width = swapChain.GetSwapChainSize().width;
		height = swapChain.GetSwapChainSize().height;
		channel = GetChannelCount(format);
		aniso = 0;
		mipLevel = 1;

		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = img;
		createInfo.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = format;
		createInfo.components.r = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = aspect;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		auto result = vkCreateImageView(ctx->GetDevice(), &createInfo, nullptr, &imgView);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
		{
			SH_ERROR_FORMAT("Failed to vkCreateImageView(): {}", string_VkResult(result));
			throw std::runtime_error(std::string{ "vkCreateImageView()" } + string_VkResult(result));
		}
		bSwapChainImg = true;
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

	SH_RENDER_API void VulkanImageBuffer::BarrierCommand(
		VkCommandBuffer cmd, 
		const VulkanImageBuffer& img, 
		VkImageLayout oldLayout, 
		VkImageLayout newLayout, 
		VkPipelineStageFlags srcStage, 
		VkPipelineStageFlags dstStage, 
		VkAccessFlags srcAccess, 
		VkAccessFlags dstAccess)
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = img.GetImage();

		barrier.subresourceRange.aspectMask = img.GetAspect();
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = img.GetMipLevel();
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		barrier.srcAccessMask = srcAccess;
		barrier.dstAccessMask = dstAccess;

		vkCmdPipelineBarrier(
			cmd,
			srcStage, dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
	}
	SH_RENDER_API auto VulkanImageBuffer::ConvertTextureFormat(TextureFormat format) -> VkFormat
	{
		switch (format)
		{
		case TextureFormat::SRGB24: [[fallthrough]]; // 대부분 지원 안 함
		case TextureFormat::SRGBA32:
			return VkFormat::VK_FORMAT_R8G8B8A8_SRGB;
		case TextureFormat::SBGR24: [[fallthrough]]; // 대부분 지원 안 함
		case TextureFormat::SBGRA32:
			return VkFormat::VK_FORMAT_B8G8R8A8_SRGB;
		case TextureFormat::RGB24: [[fallthrough]]; // 대부분 지원 안 함
		case TextureFormat::RGBA32:
			return VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
		case TextureFormat::BGR24: [[fallthrough]]; // 대부분 지원 안 함
		case TextureFormat::BGRA32:
			return VkFormat::VK_FORMAT_B8G8R8A8_UNORM;
		case TextureFormat::R8:
			return VkFormat::VK_FORMAT_R8_UNORM;
		case TextureFormat::D32S8:
			return VkFormat::VK_FORMAT_D32_SFLOAT_S8_UINT;
		case TextureFormat::D24S8:
			return VkFormat::VK_FORMAT_D24_UNORM_S8_UINT;
		case TextureFormat::D16S8:
			return VkFormat::VK_FORMAT_D16_UNORM_S8_UINT;
		default:
			return VkFormat::VK_FORMAT_UNDEFINED;
		}
	}
	auto VulkanImageBuffer::GetChannelCount(VkFormat format) -> uint32_t
	{
		switch (format)
		{
		case VkFormat::VK_FORMAT_R8G8B8A8_UNORM: [[fallthrough]];
		case VkFormat::VK_FORMAT_R8G8B8A8_SRGB: [[fallthrough]];
		case VkFormat::VK_FORMAT_B8G8R8A8_UNORM: [[fallthrough]];
		case VkFormat::VK_FORMAT_B8G8R8A8_SRGB: [[fallthrough]];
			return 4;
		case VkFormat::VK_FORMAT_D32_SFLOAT_S8_UINT: [[fallthrough]];
		case VkFormat::VK_FORMAT_D24_UNORM_S8_UINT: [[fallthrough]];
		case VkFormat::VK_FORMAT_D16_UNORM_S8_UINT: [[fallthrough]];
		case VkFormat::VK_FORMAT_R8_UNORM:
			return 1;
		default:
			return 0;
		}
	}
	auto VulkanImageBuffer::IsDepthTexture(TextureFormat format) -> bool
	{
		if (format == TextureFormat::D32S8 ||
			format == TextureFormat::D24S8 ||
			format == TextureFormat::D16S8)
			return true;
		return false;
	}
}