#include "VulkanRenderPass.h"
#include "VulkanContext.h"

#include "Core/Util.h"

#include <array>

namespace sh::render::vk
{
	SH_RENDER_API auto VulkanRenderPass::Config::operator==(const Config& other) const -> bool
	{
		return (format == other.format) &&
			(depthFormat == other.depthFormat) &&
			(bOffScreen == other.bOffScreen) &&
			(bUseStencil == other.bUseStencil) &&
			(bTransferSrc == other.bTransferSrc);
	}
	SH_RENDER_API auto VulkanRenderPass::ConfigHasher::operator()(const Config& config) const -> std::size_t
	{
		std::hash<int> intHash;
		std::hash<bool> boolHash;

		std::size_t hash = 0;
		hash = core::Util::CombineHash(intHash(config.format), intHash(config.depthFormat));
		hash = core::Util::CombineHash(hash, boolHash(config.bOffScreen));
		hash = core::Util::CombineHash(hash, boolHash(config.bUseStencil));
		hash = core::Util::CombineHash(hash, boolHash(config.bTransferSrc));
		return hash;
	}
	VulkanRenderPass::VulkanRenderPass(const VulkanContext& context) :
		context(context)
	{
	}
	VulkanRenderPass::VulkanRenderPass(VulkanRenderPass&& other) noexcept :
		context(other.context),
		renderPass(other.renderPass),
		bOffscreen(other.bOffscreen), bUseStencil(other.bUseStencil), bTransferSrc(other.bTransferSrc)
	{
		other.renderPass = VK_NULL_HANDLE;
	}
	VulkanRenderPass::~VulkanRenderPass()
	{
		Clear();
	}
	auto VulkanRenderPass::operator=(VulkanRenderPass&& other) noexcept -> VulkanRenderPass&
	{
		renderPass = other.renderPass;
		bOffscreen = other.bOffscreen;
		bUseStencil = other.bUseStencil;
		bTransferSrc = other.bTransferSrc;

		other.renderPass = VK_NULL_HANDLE;

		return *this;
	}
	void VulkanRenderPass::Create(const Config& config)
	{
		this->bOffscreen = config.bOffScreen;
		this->bUseStencil = config.bUseStencil;
		this->bTransferSrc = config.bTransferSrc;

		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = config.format;
		colorAttachment.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
		if (bTransferSrc)
			colorAttachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		else
			colorAttachment.finalLayout = bOffscreen ? 
				VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = config.depthFormat;
		depthAttachment.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
		depthAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		//렌더패스 중 ColorAttachment는 색상 첨부 전용 최적화 레이아웃이 된다. (픽셀 셰이더 출력)
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0; //VkRenderPassCreateInfo의 pAttachments에 해당하는 인덱스
		colorAttachmentRef.layout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		//깊이 attachment
		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		std::array<VkSubpassDependency, 2> dependencies;
		if (bOffscreen)
		{
			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL; // 외부 작업
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = // 해당 단계가 마무리 되야함
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[0].dstStageMask = // 이 단계 시작전에
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependencies[0].srcAccessMask =  // 해당 작업이 완료 되야함
				VkAccessFlagBits::VK_ACCESS_NONE_KHR;
			dependencies[0].dstAccessMask = // 이 작업 전에
				VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
				VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
				VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
				VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			dependencies[1].srcSubpass = 0;
			dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[1].srcStageMask =
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependencies[1].dstStageMask =
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[1].srcAccessMask =
				VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
				VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
				VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
				VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[1].dstAccessMask =
				VkAccessFlagBits::VK_ACCESS_MEMORY_READ_BIT;
			dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		}
		else
		{
			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = 
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependencies[0].dstStageMask = 
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependencies[0].srcAccessMask = 
				VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[0].dstAccessMask = 
				VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
				VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
			dependencies[0].dependencyFlags = 0;

			dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[1].dstSubpass = 0;
			dependencies[1].srcStageMask = 
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[1].dstStageMask = 
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[1].srcAccessMask = 0;
			dependencies[1].dstAccessMask = 
				VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | 
				VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
			dependencies[1].dependencyFlags = 0;
		}

		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
		renderPassInfo.pDependencies = dependencies.data();

		VkResult result = vkCreateRenderPass(context.GetDevice(), &renderPassInfo, nullptr, &renderPass);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			throw std::runtime_error(std::string{ "Failed vkCreateRenderPass()" } + string_VkResult(result));
	}
	SH_RENDER_API void VulkanRenderPass::Clear()
	{
		if (renderPass == VK_NULL_HANDLE)
			return;

		vkDestroyRenderPass(context.GetDevice(), renderPass, nullptr);
		renderPass = VK_NULL_HANDLE;
	}
	SH_RENDER_API auto sh::render::vk::VulkanRenderPass::GetVkRenderPass() const -> VkRenderPass
	{
		return renderPass;
	}
}//namespace