#include "VulkanImpl/VulkanFramebuffer.h"

#include "VulkanImpl/VulkanPipeline.h"

#include <cassert>

namespace sh::render::impl
{
	VulkanFramebuffer::VulkanFramebuffer(VkDevice device) :
		device(device), renderPass(nullptr),
		framebuffer(nullptr), img(nullptr),
		width(0), height(0)
	{
	}

	VulkanFramebuffer::VulkanFramebuffer(const VulkanFramebuffer& other) :
		device(other.device),
		framebuffer(nullptr), img(other.img), renderPass(other.renderPass),
		width(other.width), height(other.height)
	{
		if (other.framebuffer)
			Create(width, height, img, renderPass);
	}

	VulkanFramebuffer::VulkanFramebuffer(VulkanFramebuffer&& other) noexcept :
		device(other.device),
		framebuffer(other.framebuffer), img(other.img), renderPass(other.renderPass),
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
		renderPass = other.renderPass;
		if (other.framebuffer)
			Create(width, height, img, renderPass);

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

	auto VulkanFramebuffer::Create(uint32_t width, uint32_t height, VkImageView img, VkRenderPass renderPass) -> VkResult
	{
		this->renderPass = renderPass;
		this->img = img;
		this->width = width;
		this->height = height;
		VkResult result;

		VkImageView views[] = {
			img
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.pNext = nullptr;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = views;
		framebufferInfo.width = width;
		framebufferInfo.height = height;
		framebufferInfo.layers = 1;

		result = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer);
		assert(result == VkResult::VK_SUCCESS);
		return result;
	}

	void VulkanFramebuffer::Clean()
	{
		if (framebuffer)
		{
			vkDestroyFramebuffer(device, framebuffer, nullptr);
			framebuffer = nullptr;
		}
	}

	auto VulkanFramebuffer::GetVkFramebuffer() const -> VkFramebuffer
	{
		return framebuffer;
	}
}