#include "VulkanImpl/VulkanFramebuffer.h"

#include "VulkanImpl/VulkanPipeline.h"

#include <cassert>

namespace sh::render::impl
{
	VulkanFramebuffer::VulkanFramebuffer(const VulkanPipeline& pipeline) :
		pipeline(pipeline),
		framebuffer(nullptr), img(nullptr),
		width(0), height(0)
	{
	}

	VulkanFramebuffer::VulkanFramebuffer(const VulkanFramebuffer& other) :
		pipeline(other.pipeline),
		framebuffer(nullptr), img(other.img),
		width(other.width), height(other.height)
	{
		if (other.framebuffer)
			Create(width, height, img);
	}

	VulkanFramebuffer::VulkanFramebuffer(VulkanFramebuffer&& other) noexcept :
		pipeline(other.pipeline), 
		framebuffer(other.framebuffer), img(other.img),
		width(other.width), height(other.height)
	{
		other.framebuffer = nullptr;
	}

	VulkanFramebuffer::~VulkanFramebuffer()
	{
		Clean();
	}

	auto VulkanFramebuffer::operator=(const VulkanFramebuffer& other)->VulkanFramebuffer&
	{
		Clean();

		width = other.width;
		height = other.height;
		img = other.img;
		if (other.framebuffer)
			Create(width, height, img);

		return *this;
	}

	auto VulkanFramebuffer::operator=(VulkanFramebuffer&& other) noexcept -> VulkanFramebuffer&
	{
		Clean();
		framebuffer = other.framebuffer;
		other.framebuffer = nullptr;
		width = other.width;
		height = other.height;
		img = other.img;

		return *this;
	}

	auto VulkanFramebuffer::Create(uint32_t width, uint32_t height, VkImageView img) -> VkResult
	{
		VkResult result;

		VkImageView views[] = {
			img
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = pipeline.GetRenderPass();
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = views;
		framebufferInfo.width = width;
		framebufferInfo.height = height;
		framebufferInfo.layers = 1;

		result = vkCreateFramebuffer(pipeline.GetDevice(), &framebufferInfo, nullptr, &framebuffer);
		assert(result == VkResult::VK_SUCCESS);
		return result;
	}

	void VulkanFramebuffer::Clean()
	{
		if (framebuffer)
		{
			vkDestroyFramebuffer(pipeline.GetDevice(), framebuffer, nullptr);
			framebuffer = nullptr;
		}
	}

	auto VulkanFramebuffer::GetVkFramebuffer() const -> VkFramebuffer
	{
		return framebuffer;
	}
}