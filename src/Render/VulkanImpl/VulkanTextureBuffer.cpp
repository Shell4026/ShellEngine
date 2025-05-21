#include "VulkanTextureBuffer.h"
#include "VulkanContext.h"
#include "VulkanBuffer.h"
#include "VulkanFramebuffer.h"
#include "VulkanQueueManager.h"

#include <cmath>
#include <utility>
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
		width(other.width), height(other.height)
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

	void VulkanTextureBuffer::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t mipLevel)
	{
		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = mipLevel;
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

	SH_RENDER_API void VulkanTextureBuffer::Create(const IRenderContext& context, const CreateInfo& info)
	{
		this->context = &static_cast<const VulkanContext&>(context);
		queueManager = &this->context->GetQueueManager();

		isRenderTexture = false;
		width = info.width;
		height = info.height;
		uint32_t mipLevels = info.bGenerateMipmap ? static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1 : 1;
		switch (info.format)
		{
		case Texture::TextureFormat::SRGB24: [[fallthrough]]; // 일부 GPU는 3채널을 지원하지 않음
		case Texture::TextureFormat::SRGBA32:
			format = VkFormat::VK_FORMAT_R8G8B8A8_SRGB;
			channel = 4;
			break;
		case Texture::TextureFormat::RGB24: [[fallthrough]]; // 일부 GPU는 3채널을 지원하지 않음
		case Texture::TextureFormat::RGBA32:
			format = VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
			channel = 4;
			break;
		}
		if (imgBuffer == nullptr)
			imgBuffer = std::make_unique<VulkanImageBuffer>(*this->context);
		imgBuffer->SetAnisotropy(info.aniso);
		imgBuffer->Create(width, height, format,
			VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT |
			VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT,
			VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
			VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
			mipLevels);
	}
	SH_RENDER_API void VulkanTextureBuffer::Create(const Framebuffer& framebuffer)
	{
		isRenderTexture = true;
		this->framebuffer = &framebuffer;
	}

	SH_RENDER_API void VulkanTextureBuffer::SetData(const void* data, uint32_t mipLevel)
	{
		VulkanBuffer stagingBuffer{ *context };

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
		
		if (cmd == nullptr)
			cmd = std::make_unique<VulkanCommandBuffer>(*context, VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT);
		cmd->Create(context->GetCommandPool(core::ThreadType::Game));

		cmd->Build([&]
			{
				imgBuffer->ChangeLayoutCommand(cmd->GetCommandBuffer(), VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

				CopyBufferToImage(stagingBuffer.GetBuffer(), imgBuffer->GetImage(), mipWidth, mipHeight, mipLevel);

				imgBuffer->ChangeLayoutCommand(cmd->GetCommandBuffer(), VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
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
		return width * height * channel;
	}
}