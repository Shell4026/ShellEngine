#include "VulkanTextureBuffer.h"
#include "VulkanContext.h"
#include "VulkanBuffer.h"
#include "VulkanFramebuffer.h"
#include "VulkanQueueManager.h"

namespace sh::render::vk
{
	VulkanTextureBuffer::VulkanTextureBuffer() :
		isRenderTexture(false), framebuffer(nullptr)
	{
	}
	VulkanTextureBuffer::VulkanTextureBuffer(VulkanTextureBuffer&& other) noexcept :
		context(other.context), queueManager(other.queueManager),
		imgBuffer(std::move(other.imgBuffer)), cmd(std::move(other.cmd)),
		isRenderTexture(other.isRenderTexture), framebuffer(other.framebuffer),
		width(other.width), height(other.height), size(other.size)
	{
		other.context = nullptr;
		other.queueManager = nullptr;

		other.framebuffer = nullptr;
	}
	VulkanTextureBuffer::~VulkanTextureBuffer()
	{
	}
	SH_RENDER_API void VulkanTextureBuffer::Clean()
	{
		isRenderTexture = false;
		imgBuffer.reset();
		cmd.reset();
		framebuffer = nullptr;
	}

	void VulkanTextureBuffer::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = 
		{
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(cmd->GetCommandBuffer(), buffer, image, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	}

	void VulkanTextureBuffer::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = imgBuffer->GetImage();
		barrier.subresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = 0;

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
			throw std::invalid_argument("Unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			cmd->GetCommandBuffer(),
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
	}

	SH_RENDER_API void VulkanTextureBuffer::Create(const IRenderContext& context, uint32_t width, uint32_t height, Texture::TextureFormat format)
	{
		this->context = &static_cast<const VulkanContext&>(context);

		isRenderTexture = false;
		size = width * height * 4;
		this->width = width;
		this->height = height;
		switch (format)
		{
		case Texture::TextureFormat::SRGBA32:
			this->format = VkFormat::VK_FORMAT_R8G8B8A8_SRGB;
			break;
		case Texture::TextureFormat::SRGB24:
			this->format = VkFormat::VK_FORMAT_R8G8B8A8_SRGB;
			break;
		case Texture::TextureFormat::RGBA32:
			this->format = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
			break;
		case Texture::TextureFormat::RGB24:
			this->format = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
			break;
		}
		if (imgBuffer == nullptr)
			imgBuffer = std::make_unique<VulkanImageBuffer>(*this->context);
		imgBuffer->Create(width, height, this->format,
			VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT);

		queueManager = &this->context->GetQueueManager();
	}
	SH_RENDER_API void VulkanTextureBuffer::Create(const Framebuffer& framebuffer)
	{
		isRenderTexture = true;
		this->framebuffer = &framebuffer;
	}

	SH_RENDER_API void VulkanTextureBuffer::SetData(const void* data)
	{
		VulkanBuffer stagingBuffer{ *context };
		stagingBuffer.Create(size,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		stagingBuffer.SetData(data);
		
		if (cmd == nullptr)
			cmd = std::make_unique<VulkanCommandBuffer>(*context, VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT);
		cmd->Create(context->GetCommandPool(core::ThreadType::Game));

		cmd->Build([&]
			{
				TransitionImageLayout(imgBuffer->GetImage(),
					format,
					VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED,
					VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
				CopyBufferToImage(stagingBuffer.GetBuffer(), imgBuffer->GetImage(), width, height);
				TransitionImageLayout(imgBuffer->GetImage(),
					format,
					VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
		);

		assert(queueManager != nullptr);
		queueManager->SubmitCommand(*cmd);

		cmd->Reset();
	}

	SH_RENDER_API auto VulkanTextureBuffer::GetImageBuffer() const -> VulkanImageBuffer*
	{
		if(!isRenderTexture)
			return imgBuffer.get();
		else
			return static_cast<const VulkanFramebuffer*>(framebuffer)->GetColorImg();
	}
	SH_RENDER_API auto VulkanTextureBuffer::GetSize() const -> std::size_t
	{
		return size;
	}
}