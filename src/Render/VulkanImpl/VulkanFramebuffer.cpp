#include "VulkanFramebuffer.h"
#include "VulkanCommandBuffer.h"
#include "VulkanQueueManager.h"
#include "VulkanRenderPass.h"
#include "VulkanContext.h"

#include <array>
#include <cassert>

namespace sh::render::vk
{
	SH_RENDER_API VulkanFramebuffer::VulkanFramebuffer(const VulkanContext& context) :
		context(context),
		device(context.GetDevice()), gpu(context.GetGPU()), alloc(context.GetAllocator()),
		framebuffer(nullptr), img(nullptr),
		colorImg(nullptr), depthImg(nullptr),
		width(0), height(0), format(VkFormat::VK_FORMAT_R8G8B8A8_SRGB)
	{
	}

	SH_RENDER_API VulkanFramebuffer::VulkanFramebuffer(VulkanFramebuffer&& other) noexcept :
		context(other.context),
		device(other.device), gpu(other.gpu), alloc(other.alloc),
		framebuffer(other.framebuffer), img(other.img), renderPass(other.renderPass),
		colorImg(std::move(other.colorImg)), depthImg(std::move(other.depthImg)),
		width(other.width), height(other.height), format(other.format),
		bTransferSrc(other.bTransferSrc), bOffScreen(other.bOffScreen)
	{
		other.renderPass = nullptr;
		other.framebuffer = nullptr;
		other.img = nullptr;
		other.bTransferSrc = false;
		other.bOffScreen = false;
	}

	SH_RENDER_API VulkanFramebuffer::~VulkanFramebuffer()
	{
		Clean();
	}

	SH_RENDER_API auto VulkanFramebuffer::operator=(VulkanFramebuffer&& other) noexcept -> VulkanFramebuffer&
	{
		Clean();

		device = other.device;
		gpu = other.gpu;
		alloc = other.alloc;

		framebuffer = other.framebuffer;
		renderPass = other.renderPass;
		other.framebuffer = nullptr;
		other.renderPass = nullptr;

		colorImg = std::move(other.colorImg);
		depthImg = std::move(other.depthImg);

		width = other.width;
		height = other.height;
		img = other.img;
		format = other.format;
		bTransferSrc = other.bTransferSrc;

		return *this;
	}

	SH_RENDER_API auto VulkanFramebuffer::Create(const VulkanRenderPass& renderPass, uint32_t width, uint32_t height, VkImageView img, VkFormat format) -> VkResult
	{
		this->format = format;
		this->img = img;
		this->width = width;
		this->height = height;
		this->renderPass = &renderPass;

		CreateDepthBuffer();

		VkResult result;

		std::array<VkImageView, 2> views = {
			img, depthImg->GetImageView()
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.pNext = nullptr;
		framebufferInfo.renderPass = renderPass.GetVkRenderPass();
		framebufferInfo.attachmentCount = bUseDepth ? static_cast<uint32_t>(views.size()) : 1;
		framebufferInfo.pAttachments = views.data();
		framebufferInfo.width = width;
		framebufferInfo.height = height;
		framebufferInfo.layers = 1;

		result = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer);
		assert(result == VkResult::VK_SUCCESS);
		return result;
	}

	SH_RENDER_API auto VulkanFramebuffer::CreateOffScreen(const VulkanRenderPass& renderPass, uint32_t width, uint32_t height, VkFormat format, bool bTransferSrc) -> VkResult
	{
		this->width = width;
		this->height = height;
		this->bTransferSrc = bTransferSrc;
		this->format = format;
		this->renderPass = &renderPass;
		bOffScreen = true;

		VkImageUsageFlags usage = 
			VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | 
			VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT;
		if (bTransferSrc) 
			usage |= VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		colorImg = std::make_unique<VulkanImageBuffer>(context);
		auto result = colorImg->Create(width, height, format, usage,
			VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT);
		assert(result == VkResult::VK_SUCCESS);

		if (bUseDepth)
			CreateDepthBuffer();

		std::array<VkImageView, 2> views = {
			colorImg->GetImageView(), depthImg->GetImageView()
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.pNext = nullptr;
		framebufferInfo.renderPass = renderPass.GetVkRenderPass();
		framebufferInfo.attachmentCount = bUseDepth ? static_cast<uint32_t>(views.size()) : 1;
		framebufferInfo.pAttachments = views.data();
		framebufferInfo.width = width;
		framebufferInfo.height = height;
		framebufferInfo.layers = 1;

		result = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer);
		assert(result == VkResult::VK_SUCCESS);
		return result;
	}

	void VulkanFramebuffer::CreateDepthBuffer()
	{
		VkFormat depthFormat = context.FindSupportedDepthFormat(bUseStencil);
		depthImg = std::make_unique<VulkanImageBuffer>(context);
		if(width == 0 || height == 0)
			return;

		auto result = depthImg->Create(width, height, depthFormat,
			VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT);

		assert(result == VkResult::VK_SUCCESS);
	}

	SH_RENDER_API void VulkanFramebuffer::Clean()
	{
		depthImg.reset();
		colorImg.reset();

		if (framebuffer)
		{
			vkDestroyFramebuffer(device, framebuffer, nullptr);
			framebuffer = nullptr;
		}
	}
	SH_RENDER_API void VulkanFramebuffer::TransferImageToBuffer(VulkanCommandBuffer* cmd, VkBuffer buffer, int x, int y)
	{
		VkImageMemoryBarrier barrier = {};
		barrier.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT;
		if (!bOffScreen)
			barrier.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		else
			barrier.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = colorImg->GetImage();
		barrier.subresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkImageMemoryBarrier toColorAttachmentBarrier = {};
		toColorAttachmentBarrier.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		toColorAttachmentBarrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT;
		toColorAttachmentBarrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		toColorAttachmentBarrier.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		if (!bOffScreen)
			toColorAttachmentBarrier.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		else
			toColorAttachmentBarrier.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		toColorAttachmentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		toColorAttachmentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		toColorAttachmentBarrier.image = colorImg->GetImage();
		toColorAttachmentBarrier.subresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
		toColorAttachmentBarrier.subresourceRange.baseMipLevel = 0;
		toColorAttachmentBarrier.subresourceRange.levelCount = 1;
		toColorAttachmentBarrier.subresourceRange.baseArrayLayer = 0;
		toColorAttachmentBarrier.subresourceRange.layerCount = 1;

		cmd->Build([&]
		{
			if (!bTransferSrc)
			{
				vkCmdPipelineBarrier(
					cmd->GetCommandBuffer(),
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
					0, 0, nullptr, 0, nullptr, 1, &barrier
				);
			}

			VkBufferImageCopy region{};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = { x, y, 0 };
			region.imageExtent = { 1, 1, 1 };

			vkCmdCopyImageToBuffer(cmd->GetCommandBuffer(),
				colorImg->GetImage(),
				VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				buffer, 1, &region
			);

			vkCmdPipelineBarrier(
				cmd->GetCommandBuffer(),
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				0, 0, nullptr, 0, nullptr, 1, &toColorAttachmentBarrier
			);
		});

		context.GetQueueManager().SubmitCommand(*cmd);
	}

	SH_RENDER_API auto VulkanFramebuffer::UseDepthBuffer(bool bUse) -> VulkanFramebuffer&
	{
		bUseDepth = bUse;
		return *this;
	}
	SH_RENDER_API auto VulkanFramebuffer::UseStencilBuffer(bool bUse) -> VulkanFramebuffer&
	{
		bUseStencil = bUse;
		return *this;
	}

	SH_RENDER_API auto VulkanFramebuffer::GetRenderPass() const -> const VulkanRenderPass*
	{
		return renderPass;
	}

	SH_RENDER_API auto VulkanFramebuffer::GetVkFramebuffer() const -> VkFramebuffer
	{
		return framebuffer;
	}

	SH_RENDER_API auto VulkanFramebuffer::GetColorImg() const -> VulkanImageBuffer*
	{
		return colorImg.get();
	}
	SH_RENDER_API auto VulkanFramebuffer::GetDepthImg() const -> VulkanImageBuffer*
	{
		return depthImg.get();
	}
	SH_RENDER_API auto VulkanFramebuffer::GetWidth() const -> uint32_t
	{
		return width;
	}
	SH_RENDER_API auto VulkanFramebuffer::GetHeight() const -> uint32_t
	{
		return height;
	}
}