#include "VulkanImpl/VulkanFramebuffer.h"

#include "VulkanImpl/VulkanPipeline.h"

#include <cassert>

namespace sh::render::impl
{
	VulkanFramebuffer::VulkanFramebuffer(VkDevice device) :
		device(device), renderPass(nullptr),
		framebuffer(nullptr), img(nullptr),
		width(0), height(0), format(VkFormat::VK_FORMAT_R8G8B8A8_SRGB)
	{
	}

	VulkanFramebuffer::VulkanFramebuffer(const VulkanFramebuffer& other) :
		device(other.device),
		framebuffer(nullptr), img(other.img), renderPass(nullptr),
		width(other.width), height(other.height), format(other.format)
	{
		if (other.framebuffer)
			Create(width, height, img, format);
	}

	VulkanFramebuffer::VulkanFramebuffer(VulkanFramebuffer&& other) noexcept :
		device(other.device),
		framebuffer(other.framebuffer), img(other.img), renderPass(other.renderPass),
		width(other.width), height(other.height), format(other.format)
	{
		other.renderPass = nullptr;
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
		format = other.format;
		if (other.framebuffer)
			Create(width, height, img, format);

		return *this;
	}

	auto VulkanFramebuffer::operator=(VulkanFramebuffer&& other) noexcept -> VulkanFramebuffer&
	{
		Clean();
		framebuffer = other.framebuffer;
		renderPass = other.renderPass;
		other.framebuffer = nullptr;
		other.renderPass = nullptr;

		width = other.width;
		height = other.height;
		img = other.img;
		format = other.format;

		return *this;
	}

	void VulkanFramebuffer::CreateRenderPass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = format;
		colorAttachment.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
		//스텐실
		colorAttachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		//픽셀 셰이더에서 출력되는 attachment
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		//색상 출력 단계에서 대기
		dependency.srcStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		//색상 출력 단계에서 쓸 수 있을 때까지 서브 패스 전환X
		dependency.dstStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VkResult result = vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			throw std::exception{ "Can't create RenderPass" };
	}

	auto VulkanFramebuffer::Create(uint32_t width, uint32_t height, VkImageView img, VkFormat format) -> VkResult
	{
		this->format = format;
		this->img = img;
		this->width = width;
		this->height = height;

		CreateRenderPass();

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

		if (renderPass)
		{
			vkDestroyRenderPass(device, renderPass, nullptr);
			renderPass = nullptr;
		}
	}

	auto VulkanFramebuffer::GetRenderPass() const -> VkRenderPass
	{
		return renderPass;
	}

	auto VulkanFramebuffer::GetVkFramebuffer() const -> VkFramebuffer
	{
		return framebuffer;
	}
}