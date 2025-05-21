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
			(sampleCount == other.sampleCount) &&
			(bOffScreen == other.bOffScreen) &&
			(bUseDepth == other.bUseDepth) &&
			(bUseStencil == other.bUseStencil) &&
			(bTransferSrc == other.bTransferSrc) &&
			(bClear == other.bClear);
	}
	SH_RENDER_API auto VulkanRenderPass::ConfigHasher::operator()(const Config& config) const -> std::size_t
	{
		std::hash<int> intHash;
		std::hash<bool> boolHash;

		std::size_t hash = 0;
		hash = core::Util::CombineHash(intHash(config.format), intHash(config.depthFormat));
		hash = core::Util::CombineHash(hash, intHash(config.sampleCount));
		hash = core::Util::CombineHash(hash, boolHash(config.bOffScreen));
		hash = core::Util::CombineHash(hash, boolHash(config.bUseDepth));
		hash = core::Util::CombineHash(hash, boolHash(config.bUseStencil));
		hash = core::Util::CombineHash(hash, boolHash(config.bTransferSrc));
		hash = core::Util::CombineHash(hash, boolHash(config.bClear));
		return hash;
	}
	auto VulkanRenderPass::GetOffScreenSubPassDependency() const -> std::array<VkSubpassDependency, 2>
	{
		std::array<VkSubpassDependency, 2> dependencies;
		// 셰이더에서 오프스크린 텍스처 읽기 완료 -> 렌더링 쓰기 작업 시작
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL; // 외부 작업
		dependencies[0].dstSubpass = 0;

		dependencies[0].srcStageMask = // 해당 단계가 마무리 되야함
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		dependencies[0].dstStageMask = // 이 단계 시작전에
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

		dependencies[0].srcAccessMask = // 해당 작업이 완료 되야함
			VkAccessFlagBits::VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = // 이 작업 전에
			VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
			VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		dependencies[0].dependencyFlags = VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT;

		// 렌더링 결과 완전히 기록 -> 다음 셰이더에서 안전하게 샘플링 가능
		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;

		dependencies[1].srcStageMask =
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[1].dstStageMask =
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

		dependencies[1].srcAccessMask =
			VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
			VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask =
			VkAccessFlagBits::VK_ACCESS_MEMORY_READ_BIT;

		dependencies[1].dependencyFlags = VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT;

		return dependencies;
	}
	auto VulkanRenderPass::GetOnScreenSubPassDependency() const->std::array<VkSubpassDependency, 2>
	{
		std::array<VkSubpassDependency, 2> dependencies;
		// 깊이 버퍼
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL; // 외부 작업
		dependencies[0].dstSubpass = 0;

		dependencies[0].srcStageMask = // 해당 단계가 마무리 되야함
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; // 초기 상태
		dependencies[0].dstStageMask = // 이 단계 시작전에
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

		dependencies[0].srcAccessMask =  // 해당 작업이 완료 되야함
			VkAccessFlagBits::VK_ACCESS_NONE;
		dependencies[0].dstAccessMask = // 이 작업 전에
			VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
			VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

		dependencies[0].dependencyFlags = 0; // 전체 영역 동기화 필요

		// 컬러 버퍼
		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;

		dependencies[1].srcStageMask =
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask =
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		dependencies[1].srcAccessMask =
			VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		dependencies[1].dstAccessMask =
			VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		dependencies[1].dependencyFlags = 0;

		return dependencies;
	}

	VulkanRenderPass::VulkanRenderPass(const VulkanContext& context) :
		context(context)
	{
	}
	VulkanRenderPass::VulkanRenderPass(VulkanRenderPass&& other) noexcept :
		context(other.context),
		renderPass(other.renderPass),
		config(other.config)
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
		config = other.config;

		other.renderPass = VK_NULL_HANDLE;

		return *this;
	}
	void VulkanRenderPass::Create(const Config& _config)
	{
		config = _config;
		if (_config.bUseStencil)
			config.bUseDepth = true;

		bool bMSAA = config.sampleCount != VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;

		// 렌더패스 중 ColorAttachment는 색상 첨부 전용 최적화 레이아웃이 된다. (픽셀 셰이더 출력)
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0; //VkRenderPassCreateInfo의 pAttachments에 해당하는 인덱스
		colorAttachmentRef.layout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		// 깊이 attachment
		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = bMSAA ? 2 : 1;
		depthAttachmentRef.layout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		// 멀티 샘플링용
		VkAttachmentReference resolveAttachmentRef{};
		resolveAttachmentRef.attachment = 1; //VkRenderPassCreateInfo의 pAttachments에 해당하는 인덱스
		resolveAttachmentRef.layout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = config.bUseDepth ? &depthAttachmentRef : VK_NULL_HANDLE;
		subpass.pResolveAttachments = bMSAA ? &resolveAttachmentRef : VK_NULL_HANDLE;

		std::array<VkSubpassDependency, 2> dependencies;
		if (config.bOffScreen)
			dependencies = GetOffScreenSubPassDependency();
		else
			dependencies = GetOnScreenSubPassDependency();

		VkImageLayout initialColorLayout = 
			config.bClear ? VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED
			: (bMSAA ? VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL // MSAA + no clear
				: (config.bOffScreen ? VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
					: VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR));
		finalColorLayout = 
			bMSAA ? VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			: config.bTransferSrc ? VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
			: config.bOffScreen ? VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = config.format;
		colorAttachment.samples = config.sampleCount;
		colorAttachment.loadOp = config.bClear ? VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR : VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE; // Clear하는게 성능에 좋지만, 다음 패스에서 clear를 안 하고 그대로 쓰는 경우엔 저장해야 한다.
		colorAttachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = initialColorLayout;
		colorAttachment.finalLayout = finalColorLayout;

		VkImageLayout initialResolveColorLayout =
			config.bClear ? VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED
				: config.bTransferSrc ? VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
					: (config.bOffScreen ? VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
						: VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

		VkAttachmentDescription resolveAttachment{};
		resolveAttachment.format = config.format;
		resolveAttachment.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
		resolveAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		resolveAttachment.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
		resolveAttachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		resolveAttachment.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
		resolveAttachment.initialLayout = initialResolveColorLayout;
		resolveAttachment.finalLayout =
			config.bTransferSrc ? VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL 
				: config.bOffScreen ? VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		if (bMSAA)
			finalColorLayout = resolveAttachment.finalLayout;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = config.depthFormat;
		depthAttachment.samples = config.sampleCount;
		depthAttachment.loadOp = config.bClear ? VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR : VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;
		depthAttachment.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
		if (config.bUseStencil)
		{
			depthAttachment.stencilLoadOp = config.bClear ? VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR : VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;
			depthAttachment.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
		}
		else
		{
			depthAttachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
		}
		depthAttachment.initialLayout = config.bClear ? VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED : VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthAttachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		
		std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
		std::array<VkAttachmentDescription, 3> MSAAattachments = { colorAttachment, resolveAttachment, depthAttachment };
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = bMSAA ? static_cast<uint32_t>(MSAAattachments.size()) : static_cast<uint32_t>(attachments.size());
		if (!config.bUseDepth)
			renderPassInfo.attachmentCount -= 1;
		renderPassInfo.pAttachments = bMSAA ? MSAAattachments.data() : attachments.data();
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
	SH_RENDER_API auto VulkanRenderPass::GetConfig() const -> const Config&
	{
		return config;
	}
	SH_RENDER_API auto VulkanRenderPass::GetFinalColorLayout() const -> VkImageLayout
	{
		return finalColorLayout;
	}
}//namespace