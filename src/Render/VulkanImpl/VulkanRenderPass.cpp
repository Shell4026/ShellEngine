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

		// 서브패스 시작 시점(조기 프래그먼트 테스트와 컬러 출력 단계)에서 depth/color에 대한 읽기/쓰기 접근이 허용되어야 한다라는 의미
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL; // 외부 작업
		dependencies[0].dstSubpass = 0;

		dependencies[0].srcStageMask = // 이전 프레임의 컬러 어태치먼트 출력 단계가 완료될 때까지 기다린다.
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].dstStageMask = // 현재 렌더 패스의 Early Fragment Tests 단계와 Color Attachment Output 단계가 시작되기 전에 동기화가 이뤄짐
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		dependencies[0].srcAccessMask = // 이전 작업에서 발생한 메모리 읽기 작업이 완료되었는지 확인한다.
			VkAccessFlagBits::VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = // 현재 렌더 패스에서 수행될 읽기/쓰기 작업이 안전하게 시작되도록 보장
			VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
			VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
			VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
			VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

		dependencies[0].dependencyFlags = VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT;

		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;

		dependencies[1].srcStageMask =
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependencies[1].dstStageMask =
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

		dependencies[1].srcAccessMask =
			VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
			VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
			VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
			VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask =
			VkAccessFlagBits::VK_ACCESS_NONE;

		dependencies[1].dependencyFlags = VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT;

		return dependencies;
	}

	VulkanRenderPass::VulkanRenderPass(const VulkanContext& context) :
		context(context)
	{
	}
	VulkanRenderPass::VulkanRenderPass(VulkanRenderPass&& other) noexcept :
		context(other.context),
		renderPass(other.renderPass),
		config(other.config),
		initialColorLayout(other.initialColorLayout),
		initialDepthLayout(other.initialDepthLayout),
		finalColorLayout(other.finalColorLayout),
		finalDepthLayout(other.finalDepthLayout)
	{
		other.renderPass = VK_NULL_HANDLE;
	}
	VulkanRenderPass::~VulkanRenderPass()
	{
		Clear();
	}
	auto VulkanRenderPass::operator=(VulkanRenderPass&& other) noexcept -> VulkanRenderPass&
	{
		assert(&context == &other.context);
		Clear();

		renderPass = other.renderPass;
		config = other.config;

		initialColorLayout = other.initialColorLayout;
		initialDepthLayout = other.initialDepthLayout;
		finalColorLayout = other.finalColorLayout;
		finalDepthLayout = other.finalDepthLayout;

		other.renderPass = VK_NULL_HANDLE;

		return *this;
	}
	void VulkanRenderPass::Create(const Config& _config)
	{
		Clear();

		config = _config;
		if (_config.bUseStencil)
			config.bUseDepth = true;

		const bool bMSAA = config.sampleCount != VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;

		// 렌더패스 중 ColorAttachment는 색상 첨부 전용 최적화 레이아웃이 된다. (픽셀 셰이더 출력)
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0; //VkRenderPassCreateInfo의 pAttachments에 해당하는 인덱스
		colorAttachmentRef.layout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		// 멀티 샘플링용 리졸브 어태치먼트
		VkAttachmentReference resolveAttachmentRef{};
		resolveAttachmentRef.attachment = 1; //VkRenderPassCreateInfo의 pAttachments에 해당하는 인덱스
		resolveAttachmentRef.layout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		// 깊이 attachment
		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = bMSAA ? 2 : 1;
		depthAttachmentRef.layout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

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

		if (config.bUseDepth)
		{
			initialDepthLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
			finalDepthLayout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		
		std::vector<VkAttachmentDescription> attachments;
		if (bMSAA)
			attachments = CreateMSAAAttachments();
		else
			attachments = CreateAttachments();

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = attachments.size();
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
	SH_RENDER_API auto VulkanRenderPass::GetConfig() const -> const Config&
	{
		return config;
	}
	SH_RENDER_API auto VulkanRenderPass::GetInitialColorLayout() const -> VkImageLayout
	{
		return initialColorLayout;
	}
	SH_RENDER_API auto VulkanRenderPass::GetFinalColorLayout() const -> VkImageLayout
	{
		return finalColorLayout;
	}
	SH_RENDER_API auto VulkanRenderPass::GetInitialDepthLayout() const -> VkImageLayout
	{
		return initialDepthLayout;
	}
	SH_RENDER_API auto VulkanRenderPass::GetFinalDepthLayout() const -> VkImageLayout
	{
		return finalDepthLayout;
	}
	auto VulkanRenderPass::CreateMSAAAttachments() -> std::vector<VkAttachmentDescription>
	{
		initialColorLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
		if (config.bOffScreen)
		{
			finalColorLayout = config.bTransferSrc ? VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL : VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
		else
		{
			// resolve -> swapchain
			finalColorLayout = config.bTransferSrc ? VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL : VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		}

		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = config.format;
		colorAttachment.samples = config.sampleCount;
		colorAttachment.loadOp = config.bClear ? VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR : VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE; // 원래는 초기화 하는게 맞는데 다른 곳에서 clear 안 하고 쓸 수도 있어서..
		colorAttachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = config.bClear ? VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED : VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription resolveAttachment{};
		resolveAttachment.format = config.format;
		resolveAttachment.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
		resolveAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		resolveAttachment.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
		resolveAttachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		resolveAttachment.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
		resolveAttachment.initialLayout = initialColorLayout;
		resolveAttachment.finalLayout = finalColorLayout;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = config.depthFormat;
		depthAttachment.samples = config.sampleCount;
		depthAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
		if (config.bUseStencil)
		{
			depthAttachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
		}
		else
		{
			depthAttachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
		}
		depthAttachment.initialLayout = initialDepthLayout;
		depthAttachment.finalLayout = finalDepthLayout;

		std::vector<VkAttachmentDescription> result{ colorAttachment, resolveAttachment };
		if (config.bUseDepth)
			result.push_back(depthAttachment);

		return result;
	}
	auto VulkanRenderPass::CreateAttachments() -> std::vector<VkAttachmentDescription>
	{
		if (config.bOffScreen)
		{
			initialColorLayout = config.bClear ? VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED : VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			finalColorLayout = config.bTransferSrc ? VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL : VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
		else // onscreen (swapchain)
		{
			initialColorLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
			finalColorLayout = config.bTransferSrc ? VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL : VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		}

		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = config.format;
		colorAttachment.samples = config.sampleCount;
		colorAttachment.loadOp = config.bClear ? VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR : VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = initialColorLayout;
		colorAttachment.finalLayout = finalColorLayout;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = config.depthFormat;
		depthAttachment.samples = config.sampleCount;
		depthAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
		if (config.bUseStencil)
		{
			depthAttachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
		}
		else
		{
			depthAttachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
		}
		depthAttachment.initialLayout = initialDepthLayout;
		depthAttachment.finalLayout = finalDepthLayout;

		std::vector<VkAttachmentDescription> result{ colorAttachment };
		if (config.bUseDepth)
			result.push_back(depthAttachment);

		return result;
	}
}//namespace