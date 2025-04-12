#include "VulkanRenderer.h"
#include "VulkanContext.h"
#include "VulkanSwapChain.h"
#include "VulkanQueueManager.h"
#include "VulkanPipelineManager.h"
#include "VulkanFramebuffer.h"
#include "VulkanRenderPass.h"
#include "VulkanCameraBuffers.h"

#include "Core/Util.h"

#include <cassert>
#include <cstdint>
#include <utility>

#undef min
#undef max

namespace sh::render::vk
{
	SH_RENDER_API VulkanRenderer::VulkanRenderer() :
		currentFrame(0),
		isInit(false),
		gameThreadSemaphore(nullptr)
	{
	}

	SH_RENDER_API VulkanRenderer::~VulkanRenderer()
	{
		if(isInit)
			Clear();
	}

	SH_RENDER_API void VulkanRenderer::Clear()
	{
		Renderer::Clear();

		camManager->Destroy();
		vkDeviceWaitIdle(context->GetDevice());
		DestroySyncObjects();
		context->Clean();

		isInit = false;
	}

	auto VulkanRenderer::CreateSyncObjects() -> VkResult
	{
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VkFenceCreateFlagBits::VK_FENCE_CREATE_SIGNALED_BIT; //시작부터 신호를 받음

		VkResult result;
		result = vkCreateSemaphore(context->GetDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphore);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			return result;

		result = vkCreateSemaphore(context->GetDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphore);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			return result;

		result = vkCreateSemaphore(context->GetDevice(), &semaphoreInfo, nullptr, &gameThreadSemaphore);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			return result;

		result = vkCreateFence(context->GetDevice(), &fenceInfo, nullptr, &inFlightFence);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			return result;

		return result;
	}

	void VulkanRenderer::DestroySyncObjects()
	{
		vkDestroySemaphore(context->GetDevice(), imageAvailableSemaphore, nullptr);
		vkDestroySemaphore(context->GetDevice(), renderFinishedSemaphore, nullptr);
		vkDestroySemaphore(context->GetDevice(), gameThreadSemaphore, nullptr);
		vkDestroyFence(context->GetDevice(), inFlightFence, nullptr);
	}
	void VulkanRenderer::OnCameraAdded(const Camera* camera)
	{
		VulkanCameraBuffers::GetInstance()->AddCamera(*camera);
	}
	void VulkanRenderer::OnCameraRemoved(const Camera* camera)
	{
		VulkanCameraBuffers::GetInstance()->AddCamera(*camera);
	}

	SH_RENDER_API bool VulkanRenderer::Init(const sh::window::Window& win)
	{
		Renderer::Init(win);

		context = std::make_unique<VulkanContext>(win);
		context->Init();

		camManager = VulkanCameraBuffers::GetInstance();
		camManager->Init(*context);

		if (CreateSyncObjects() != VkResult::VK_SUCCESS)
			return false;

		VulkanCommandBuffer* cmd = context->GetCommandBuffer(core::ThreadType::Render);
		cmd->SetWaitSemaphore({ imageAvailableSemaphore });
		cmd->SetSignalSemaphore({ renderFinishedSemaphore });
		cmd->SetWaitStage(
			{
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
			}
		);

		isInit = true;
		return true;
	}

	SH_RENDER_API bool VulkanRenderer::Resizing()
	{
		vkDeviceWaitIdle(context->GetDevice());

		DestroySyncObjects();
		CreateSyncObjects();

		VulkanCommandBuffer* cmd = context->GetCommandBuffer(core::ThreadType::Render);
		cmd->Reset();
		cmd->SetWaitSemaphore({ imageAvailableSemaphore });
		cmd->SetSignalSemaphore({ renderFinishedSemaphore });
		cmd->SetWaitStage(
			{
				VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
			}
		);

		return context->ReSizing();
	}

	SH_RENDER_API bool VulkanRenderer::IsInit() const
	{
		return isInit;
	}

	SH_RENDER_API void VulkanRenderer::WaitForCurrentFrame()
	{
		vkWaitForFences(context->GetDevice(), 1, &inFlightFence, VK_TRUE, UINT64_MAX);
	}

	SH_RENDER_API void VulkanRenderer::Render()
	{
		if (!isInit || bPause.load(std::memory_order::memory_order_acquire))
			return;

		WaitForCurrentFrame();

		uint32_t imgIdx;
		VkResult result = vkAcquireNextImageKHR(context->GetDevice(), context->GetSwapChain().GetSwapChain(), UINT64_MAX, imageAvailableSemaphore, nullptr, &imgIdx);
		if (result == VkResult::VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			if (IsPause())
				return;
			if (result == VkResult::VK_ERROR_OUT_OF_DATE_KHR)
			{
				SH_INFO("Resizing");
				Resizing();
			}
			return;
		}
		if (result == VkResult::VK_ERROR_SURFACE_LOST_KHR)
			return;

		vkResetFences(context->GetDevice(), 1, &inFlightFence);

		VulkanCommandBuffer* cmd = context->GetCommandBuffer(core::ThreadType::Render);
		cmd->Reset();

		std::vector<const Camera*> cams{};
		cams.reserve(cameras.size());
		for (auto camera : cameras)
		{
			camManager->UploadDataToGPU(*camera);
			cams.push_back(camera);
		}

		cmd->Build([&]
		{
			uint32_t drawcall = 0;
			for (auto& renderPipeline : renderPipelines)
			{
				renderPipeline->RecordCommand(cams, imgIdx);
				drawcall += renderPipeline->GetDrawCallCount();
			}
			SetDrawCall(drawcall);

			const VulkanFramebuffer* mainFramebuffer = static_cast<const VulkanFramebuffer*>(context->GetMainFramebuffer(imgIdx));

			VkRenderPassBeginInfo renderPassInfo{};
			std::array<VkClearValue, 2> clear;
			clear[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
			clear[1].depthStencil = { 1.0f, 0 };

			std::array<VkClearValue, 3> clearMSAA;
			clearMSAA[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
			clearMSAA[1].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
			clearMSAA[2].depthStencil = { 1.0f, 0 };

			bool bMSAA = context->GetSampleCount() != VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;

			renderPassInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = mainFramebuffer->GetRenderPass()->GetVkRenderPass();
			renderPassInfo.framebuffer = mainFramebuffer->GetVkFramebuffer();
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = context->GetSwapChain().GetSwapChainSize();
			renderPassInfo.clearValueCount = bMSAA ? static_cast<uint32_t>(clearMSAA.size()) : static_cast<uint32_t>(clear.size());
			renderPassInfo.pClearValues = bMSAA ? clearMSAA.data() : clear.data();
			vkCmdBeginRenderPass(cmd->GetCommandBuffer(), &renderPassInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport{};
			float width = context->GetViewportEnd().x - context->GetViewportStart().x;
			float height = context->GetViewportEnd().y - context->GetViewportStart().y;
			float surfWidth = static_cast<float>(context->GetSwapChain().GetSwapChainSize().width);
			float surfHeight = static_cast<float>(context->GetSwapChain().GetSwapChainSize().height);
			viewport.x = context->GetViewportStart().x;
			viewport.y = context->GetViewportEnd().y;
			viewport.width = std::min(width, surfWidth);
			viewport.height = -std::min(height, surfHeight);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(cmd->GetCommandBuffer(), 0, 1, &viewport);

			VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent = context->GetSwapChain().GetSwapChainSize();
			vkCmdSetScissor(cmd->GetCommandBuffer(), 0, 1, &scissor);

			for (auto& func : drawCalls)
				func();

			vkCmdEndRenderPass(cmd->GetCommandBuffer());
		});

		context->GetQueueManager().SubmitCommand(*cmd, inFlightFence);

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
		VkSwapchainKHR swapChains[] = { context->GetSwapChain().GetSwapChain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imgIdx;
		presentInfo.pResults = nullptr;

		vkQueuePresentKHR(context->GetQueueManager().GetSurfaceQueue(), &presentInfo);
	}

	SH_RENDER_API auto VulkanRenderer::GetCurrentFrame() const -> int
	{
		return currentFrame;
	}
	SH_RENDER_API auto VulkanRenderer::GetWidth() const -> uint32_t
	{
		return context->GetSwapChain().GetSwapChainSize().width;
	}
	SH_RENDER_API auto VulkanRenderer::GetHeight() const -> uint32_t
	{
		return context->GetSwapChain().GetSwapChainSize().height;
	}
	SH_RENDER_API auto VulkanRenderer::GetRenderFinshedSemaphore() const -> VkSemaphore
	{
		return renderFinishedSemaphore;
	}
	SH_RENDER_API auto VulkanRenderer::GetGameThreadSemaphore() const -> VkSemaphore
	{
		return gameThreadSemaphore;
	}
	SH_RENDER_API auto VulkanRenderer::GetContext() const -> IRenderContext*
	{
		return context.get();
	}
	SH_RENDER_API void VulkanRenderer::Sync()
	{
		Renderer::Sync();
	}
}//namespace

