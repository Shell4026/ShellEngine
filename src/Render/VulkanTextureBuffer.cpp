#include "VulkanTextureBuffer.h"

#include "VulkanRenderer.h"
#include "VulkanImpl/VulkanBuffer.h"
#include "VulkanImpl/VulkanFramebuffer.h"

#include <stdexcept>

namespace sh::render
{
	VulkanTextureBuffer::VulkanTextureBuffer() :
		isRenderTexture(false), framebuffer(nullptr)
	{
	}
	VulkanTextureBuffer::VulkanTextureBuffer(VulkanTextureBuffer&& other) noexcept :
		buffer(std::move(other.buffer)), cmd(std::move(other.cmd)),
		isRenderTexture(other.isRenderTexture), framebuffer(other.framebuffer)
	{
		other.framebuffer = nullptr;
	}
	VulkanTextureBuffer::~VulkanTextureBuffer()
	{
	}
	void VulkanTextureBuffer::Clean()
	{
		isRenderTexture = false;
		buffer.reset();
		cmd.reset();
		framebuffer = nullptr;
	}

	void VulkanTextureBuffer::CopyBufferToImage(VkQueue queue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
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

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		cmd->Submit(queue, [&]()
		{
			vkCmdCopyBufferToImage(cmd->GetCommandBuffer(), buffer, image, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		},
		&beginInfo);
	}

	void VulkanTextureBuffer::TransitionImageLayout(VkQueue queue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = buffer->GetImage();
		barrier.subresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.srcAccessMask = 0; // TODO
		barrier.dstAccessMask = 0; // TODO

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		cmd->Submit(queue, [&]() 
		{
			vkCmdPipelineBarrier(
				cmd->GetCommandBuffer(),
				sourceStage, destinationStage,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);
		}, 
		&beginInfo);
	}

	void VulkanTextureBuffer::Create(const VulkanRenderer& renderer, const void* data, uint32_t width, uint32_t height, Texture::TextureFormat format)
	{
		isRenderTexture = false;

		buffer = std::make_unique<impl::VulkanImageBuffer>(renderer.GetDevice(), renderer.GetGPU(), renderer.GetAllocator());
		
		VkFormat vkformat = VkFormat::VK_FORMAT_R8G8B8A8_SRGB;
		size_t size = width * height * 4;

		buffer->Create(width, height, vkformat, 
			VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT);

		impl::VulkanBuffer stagingBuffer{ renderer.GetDevice(), renderer.GetGPU(), renderer.GetAllocator() };
		stagingBuffer.Create(size,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		stagingBuffer.SetData(data);

		cmd = std::make_unique<impl::VulkanCommandBuffer>(renderer.GetDevice(), renderer.GetCommandPool());
		cmd->Create();

		TransitionImageLayout(renderer.GetGraphicsQueue(), buffer->GetImage(),
			vkformat,
			VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED, 
			VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		CopyBufferToImage(renderer.GetGraphicsQueue(), stagingBuffer.GetBuffer(), buffer->GetImage(), width, height);
		TransitionImageLayout(renderer.GetGraphicsQueue(), buffer->GetImage(),
			vkformat,
			VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
	void VulkanTextureBuffer::Create(const Framebuffer& framebuffer)
	{
		isRenderTexture = true;
		this->framebuffer = &framebuffer;
	}

	auto VulkanTextureBuffer::GetImageBuffer() const -> impl::VulkanImageBuffer*
	{
		if(!isRenderTexture)
			return buffer.get();
		else
			return static_cast<const impl::VulkanFramebuffer*>(framebuffer)->GetColorImg();
	}
}