#include "VulkanImpl/VulkanFramebuffer.h"
#include "VulkanImpl/VulkanCommandBuffer.h"

#include <array>
#include <cassert>
#include <stdexcept>

namespace sh::render::vk
{
	SH_RENDER_API VulkanFramebuffer::VulkanFramebuffer(VkDevice device, VkPhysicalDevice gpu, VmaAllocator alloc) :
		device(device), gpu(gpu), alloc(alloc),
		renderPass(nullptr),
		framebuffer(nullptr), img(nullptr),
		colorImg(nullptr), depthImg(nullptr),
		width(0), height(0), format(VkFormat::VK_FORMAT_R8G8B8A8_SRGB)
	{
	}

	SH_RENDER_API VulkanFramebuffer::VulkanFramebuffer(VulkanFramebuffer&& other) noexcept :
		device(other.device), gpu(other.gpu), alloc(other.alloc),
		framebuffer(other.framebuffer), img(other.img), renderPass(other.renderPass),
		colorImg(std::move(other.colorImg)), depthImg(std::move(other.depthImg)),
		width(other.width), height(other.height), format(other.format),
		bTransferSrc(other.bTransferSrc)
	{
		other.renderPass = nullptr;
		other.framebuffer = nullptr;
		other.img = nullptr;
		other.bTransferSrc = false;
	}

	SH_RENDER_API VulkanFramebuffer::~VulkanFramebuffer()
	{
		Clean();
	}

	SH_RENDER_API auto VulkanFramebuffer::operator=(VulkanFramebuffer&& other) noexcept -> VulkanFramebuffer&
	{
		Clean();

		device = other.device;
		gpu = other.gpu;
		alloc = other.alloc;

		framebuffer = other.framebuffer;
		renderPass = other.renderPass;
		other.framebuffer = nullptr;
		other.renderPass = nullptr;

		colorImg = std::move(other.colorImg);
		depthImg = std::move(other.depthImg);

		width = other.width;
		height = other.height;
		img = other.img;
		format = other.format;
		bTransferSrc = other.bTransferSrc;

		return *this;
	}

	void VulkanFramebuffer::CreateRenderPass()
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = format;
		colorAttachment.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
		//초기 레이아웃
		colorAttachment.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
		//최종 레이아웃
		if (colorImg == nullptr)
			colorAttachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // 화면 출력용
		else //offscreen
		{
			if (!bTransferSrc)
				colorAttachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // 셰이더 읽기용
			else
				colorAttachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL; // 전송 소스용
		}

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

		//렌더패스 중 ColorAttachment는 색상 첨부 전용 최적화 레이아웃이 된다. (픽셀 셰이더 출력)
		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0; //VkRenderPassCreateInfo의 pAttachments에 해당하는 인덱스
		colorAttachmentRef.layout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		//깊이 attachment
		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		//서브패스
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		
		std::vector<VkSubpassDependency> dependencies;
		if (colorImg == nullptr)
		{
			dependencies.resize(2);
			//종속성: 외부 작업(VK_SUBPASS_EXTERNAL)이 첫 번째 서브패스의 시작 전에 완료되어야 함
			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = // 외부작업에서 픽셀 테스트 단계가 끝나야함
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | 
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependencies[0].dstStageMask = // 첫 서브패스의 픽셀 테스트 단계전에 srcStageMask가 완료돼야 함
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | 
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependencies[0].srcAccessMask = // 외부작업은 깊이/스텐실에 쓰기 작업을 수행
				VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[0].dstAccessMask = // 첫 번째 서브패스는 깊이/스텐실에 쓰기와 읽기 작업을 수행
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
		else //offscreen 렌더링
		{
			dependencies.resize(2);
			
			// 첫 번째 종속성
			// 외부의 픽셀 셰이더 단계 이후에 첫 번째 서브패스에서 컬러와 깊이/스텐실을 읽고 쓸 수 있도록 설정한다.
			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = 
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[0].srcAccessMask = 
				VkAccessFlagBits::VK_ACCESS_NONE_KHR;
			dependencies[0].dstStageMask = 
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | 
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | 
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependencies[0].dstAccessMask = 
				VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | 
				VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | 
				VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | 
				VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[0].dependencyFlags = VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT; //프레임버퍼 로컬

			// 두 번째 종속성
			// 첫 번째 서브패스의 컬러 및 깊이/스텐실 접근이 끝난 후에 외부 픽셀 셰이더 단계에서 메모리를 읽을 수 있도록 설정한다.
			dependencies[1].srcSubpass = 0;
			dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[1].srcStageMask = 
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | 
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | 
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependencies[1].srcAccessMask = 
				VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | 
				VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | 
				VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | 
				VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			if (!bTransferSrc)
			{
				dependencies[1].dstStageMask =
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				dependencies[1].dstAccessMask =
					VkAccessFlagBits::VK_ACCESS_MEMORY_READ_BIT;
			}
			else
			{
				dependencies[1].dstStageMask =
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT;
				dependencies[1].dstAccessMask =
					VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT | 
					VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
			}

			dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
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

	SH_RENDER_API auto VulkanFramebuffer::Create(uint32_t width, uint32_t height, VkImageView img, VkFormat format) -> VkResult
	{
		this->format = format;
		this->img = img;
		this->width = width;
		this->height = height;

		CreateDepthBuffer();
		CreateRenderPass();

		VkResult result;

		std::array<VkImageView, 2> views = {
			img, depthImg->GetImageView()
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

	SH_RENDER_API auto VulkanFramebuffer::CreateOffScreen(uint32_t width, uint32_t height, VkFormat format, bool bTransferSrc) -> VkResult
	{
		this->width = width;
		this->height = height;
		this->bTransferSrc = bTransferSrc;
		this->format = format;

		VkImageUsageFlags usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		if (!bTransferSrc) 
			usage |= VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT;
		else
			usage |= VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		colorImg = std::make_unique<VulkanImageBuffer>(device, gpu, alloc);
		auto result = colorImg->Create(width, height, format, usage,
			VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT);
		assert(result == VkResult::VK_SUCCESS);

		CreateDepthBuffer();
		CreateRenderPass();

		std::array<VkImageView, 2> views = {
			colorImg->GetImageView(), depthImg->GetImageView()
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

	void VulkanFramebuffer::CreateDepthBuffer()
	{
		VkFormat depthFormat = FindSupportedDepthFormat();
		depthImg = std::make_unique<VulkanImageBuffer>(device, gpu, alloc);
		if(width == 0 || height == 0)
			return;

		auto result = depthImg->Create(width, height, depthFormat,
			VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT);
		assert(result == VkResult::VK_SUCCESS);

	}

	SH_RENDER_API void VulkanFramebuffer::Clean()
	{
		depthImg.reset();
		colorImg.reset();

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
	SH_RENDER_API void VulkanFramebuffer::TransferImageToBuffer(VulkanCommandBuffer* cmd, VkQueue queue, VkBuffer buffer, int x, int y)
	{
		VkImageMemoryBarrier barrier = {};
		barrier.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.srcAccessMask = VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT;
		barrier.oldLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrier.newLayout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = colorImg->GetImage();
		barrier.subresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkImageMemoryBarrier toColorAttachmentBarrier = {};
		toColorAttachmentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		toColorAttachmentBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		toColorAttachmentBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		toColorAttachmentBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		toColorAttachmentBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		toColorAttachmentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		toColorAttachmentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		toColorAttachmentBarrier.image = colorImg->GetImage();
		toColorAttachmentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		toColorAttachmentBarrier.subresourceRange.baseMipLevel = 0;
		toColorAttachmentBarrier.subresourceRange.levelCount = 1;
		toColorAttachmentBarrier.subresourceRange.baseArrayLayer = 0;
		toColorAttachmentBarrier.subresourceRange.layerCount = 1;

		cmd->Submit(queue, [&]
		{
			if (!bTransferSrc)
			{
				vkCmdPipelineBarrier(
					cmd->GetCommandBuffer(),
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
					0, 0, nullptr, 0, nullptr, 1, &barrier
				);
			}

			VkBufferImageCopy region{};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = { x, y, 0 };
			region.imageExtent = { 1, 1, 1 };

			vkCmdCopyImageToBuffer(cmd->GetCommandBuffer(),
				colorImg->GetImage(),
				VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				buffer, 1, &region
			);

			vkCmdPipelineBarrier(
				cmd->GetCommandBuffer(),
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT,
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				0, 0, nullptr, 0, nullptr, 1, &toColorAttachmentBarrier
			);
		});
	}

	SH_RENDER_API auto VulkanFramebuffer::GetRenderPass() const -> VkRenderPass
	{
		return renderPass;
	}

	SH_RENDER_API auto VulkanFramebuffer::GetVkFramebuffer() const -> VkFramebuffer
	{
		return framebuffer;
	}

	SH_RENDER_API auto VulkanFramebuffer::GetColorImg() const -> VulkanImageBuffer*
	{
		return colorImg.get();
	}
	SH_RENDER_API auto VulkanFramebuffer::GetDepthImg() const -> VulkanImageBuffer*
	{
		return depthImg.get();
	}
	SH_RENDER_API auto VulkanFramebuffer::GetWidth() const -> uint32_t
	{
		return width;
	}
	SH_RENDER_API auto VulkanFramebuffer::GetHeight() const -> uint32_t
	{
		return height;
	}
}