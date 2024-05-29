#include "VulkanImpl/VulkanFramebuffer.h"

#include <array>
#include <cassert>
#include <stdexcept>

namespace sh::render::impl
{
	VulkanFramebuffer::VulkanFramebuffer(VkDevice device, VkPhysicalDevice gpu, VmaAllocator alloc) :
		device(device), gpu(gpu), alloc(alloc),
		renderPass(nullptr),
		framebuffer(nullptr), img(nullptr),
		depthImg(device, gpu, alloc),
		width(0), height(0), format(VkFormat::VK_FORMAT_R8G8B8A8_SRGB)
	{
	}

	VulkanFramebuffer::VulkanFramebuffer(const VulkanFramebuffer& other) :
		device(other.device), gpu(other.gpu), alloc(other.alloc),
		framebuffer(nullptr), img(other.img), renderPass(nullptr),
		depthImg(other.device, other.gpu, other.alloc),
		width(other.width), height(other.height), format(other.format)
	{
		if (other.framebuffer)
			Create(width, height, img, format);
	}

	VulkanFramebuffer::VulkanFramebuffer(VulkanFramebuffer&& other) noexcept :
		device(other.device),
		framebuffer(other.framebuffer), img(other.img), renderPass(other.renderPass),
		depthImg(std::move(other.depthImg)),
		width(other.width), height(other.height), format(other.format)
	{
		other.renderPass = nullptr;
		other.framebuffer = nullptr;
		other.img = nullptr;
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
		colorAttachmentRef.layout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		//깊이 버퍼
		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = FindSupportedDepthFormat();
		depthAttachment.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		//색상 출력, 깊이 테스트 단계에서 대기
		dependency.srcStageMask = 
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | 
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		//색상 출력, 깊이 테스트 단계에서 쓸 수 있을 때까지 서브 패스 전환X
		dependency.dstStageMask = 
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | 
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = 
			VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | 
			VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VkResult result = vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			throw std::exception();
	}

	auto VulkanFramebuffer::FindSupportedDepthFormat() -> VkFormat
	{
		std::array<VkFormat, 3> formats = { VkFormat::VK_FORMAT_D32_SFLOAT, VkFormat::VK_FORMAT_D32_SFLOAT_S8_UINT, VkFormat::VK_FORMAT_D24_UNORM_S8_UINT };

		auto feature = VkFormatFeatureFlagBits::VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
		for (VkFormat format : formats) {
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(gpu, format, &props);

			if ((props.optimalTilingFeatures & feature) == feature) {
				return format;
			}
		}

		throw std::runtime_error("Failed to find supported Depth format!");
	}

	auto VulkanFramebuffer::CreateDepthBuffer()
	{
		VkFormat depthFormat = FindSupportedDepthFormat();
		auto result = depthImg.Create(width, height, depthFormat,
			VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
			VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT);
		assert(result == VkResult::VK_SUCCESS);

	}

	auto VulkanFramebuffer::Create(uint32_t width, uint32_t height, VkImageView img, VkFormat format) -> VkResult
	{
		this->format = format;
		this->img = img;
		this->width = width;
		this->height = height;

		CreateDepthBuffer();
		CreateRenderPass();

		VkResult result;

		std::array<VkImageView, 2> views = {
			img, depthImg.GetImageView()
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.pNext = nullptr;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(views.size());
		framebufferInfo.pAttachments = views.data();
		framebufferInfo.width = width;
		framebufferInfo.height = height;
		framebufferInfo.layers = 1;

		result = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer);
		assert(result == VkResult::VK_SUCCESS);
		return result;
	}

	void VulkanFramebuffer::Clean()
	{
		depthImg.Clean();

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