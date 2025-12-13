#include "VulkanRenderer.h"
#include "VulkanContext.h"
#include "VulkanSwapChain.h"
#include "VulkanQueueManager.h"
#include "VulkanPipelineManager.h"
#include "VulkanFramebuffer.h"
#include "VulkanRenderPass.h"
#include "VulkanCameraBuffers.h"
#include "VulkanRenderPipelineImpl.h"
#include "VulkanRenderPassManager.h"
#include "VulkanCommandBufferPool.h"

#include "Core/Util.h"
#include "Core/ThreadPool.h"

#include <cassert>
#include <cstdint>
#include <utility>
#include <limits>
#include <algorithm>
#include <map>
#include <unordered_set>

#undef min
#undef max

namespace sh::render::vk
{
	SH_RENDER_API VulkanRenderer::VulkanRenderer() :
		currentFrame(0),
		isInit(false)
	{
	}

	SH_RENDER_API VulkanRenderer::~VulkanRenderer()
	{
		if (isInit)
		{
			Clear();

			camManager->Destroy();
			DestroySyncObjects();
			context->Clean();
		}
	}

	SH_RENDER_API void VulkanRenderer::Clear()
	{
		vkDeviceWaitIdle(context->GetDevice());

		context->GetCommandBufferPool().DeallocateCommandBuffer(*cmd);
		cmd = nullptr;

		frameFences.clear();
		camManager->Clear();

		Renderer::Clear();
	}

	auto VulkanRenderer::CreateSyncObjects() -> VkResult
	{
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = 0;

		VkResult result;
		result = vkCreateSemaphore(context->GetDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphore);
		if (result != VK_SUCCESS) return result;

		result = vkCreateSemaphore(context->GetDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphore);
		if (result != VK_SUCCESS) return result;

		VkSemaphoreTypeCreateInfo timelineCreateInfo{};
		timelineCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
		timelineCreateInfo.semaphoreType = VkSemaphoreType::VK_SEMAPHORE_TYPE_TIMELINE;
		timelineCreateInfo.initialValue = 0;
		semaphoreInfo.pNext = &timelineCreateInfo;

		result = vkCreateSemaphore(context->GetDevice(), &semaphoreInfo, nullptr, &timelineSemaphore);
		if (result != VK_SUCCESS) return result;

		result = vkCreateFence(context->GetDevice(), &fenceInfo, nullptr, &inFlightFence);
		if (result != VK_SUCCESS) return result;

		return VK_SUCCESS;
	}

	void VulkanRenderer::DestroySyncObjects()
	{
		vkDestroySemaphore(context->GetDevice(), imageAvailableSemaphore, nullptr);
		vkDestroySemaphore(context->GetDevice(), renderFinishedSemaphore, nullptr);
		vkDestroySemaphore(context->GetDevice(), timelineSemaphore, nullptr);
		vkDestroyFence(context->GetDevice(), inFlightFence, nullptr);
	}
	void VulkanRenderer::OnCameraAdded(const Camera* camera)
	{
		VulkanCameraBuffers::GetInstance()->AddCamera(*camera);
	}
	void VulkanRenderer::OnCameraRemoved(const Camera* camera)
	{
		VulkanCameraBuffers::GetInstance()->RemoveCamera(*camera);
	}

	SH_RENDER_API bool VulkanRenderer::Init(sh::window::Window& win)
	{
		Renderer::Init(win);

		context = std::make_unique<VulkanContext>(win);
		context->Init();

		camManager = VulkanCameraBuffers::GetInstance();
		camManager->Init(*context);

		if (CreateSyncObjects() != VkResult::VK_SUCCESS)
			return false;

		isInit = true;
		return true;
	}

	SH_RENDER_API bool VulkanRenderer::Resizing()
	{
		vkDeviceWaitIdle(context->GetDevice());

		DestroySyncObjects();
		CreateSyncObjects();

		if (cmd == nullptr)
			cmd = context->GetCommandBufferPool().AllocateCommandBuffer(std::this_thread::get_id(), VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT);
		cmd->ResetSyncObjects();
		cmd->SetSignalSemaphores({ VulkanCommandBuffer::SignalSemaphore{ renderFinishedSemaphore } });

		return context->ReSizing();
	}

	SH_RENDER_API bool VulkanRenderer::IsInit() const
	{
		return isInit;
	}

	SH_RENDER_API void VulkanRenderer::WaitForCurrentFrame()
	{
		vkDeviceWaitIdle(context->GetDevice());
	}

	SH_RENDER_API void VulkanRenderer::Render()
	{
		using SignalSemaphore = VulkanCommandBuffer::SignalSemaphore;
		using WaitSemaphore = VulkanCommandBuffer::WaitSemaphore;
		Renderer::Render();

		if (!isInit || bPause.load(std::memory_order::memory_order_acquire))
			return;

		// 스왑체인에서 이미지를 가져오고, 변경 사항이 있다면 리사이징
		uint32_t imgIdx;
		VkResult result = vkAcquireNextImageKHR(context->GetDevice(), context->GetSwapChain().GetSwapChain(), UINT64_MAX, imageAvailableSemaphore, nullptr, &imgIdx);
		if (result == VkResult::VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			if (IsPause())
				return;
			SH_INFO("Resizing");
			Resizing();
			return;
		}
		if (result == VkResult::VK_ERROR_SURFACE_LOST_KHR)
		{
			SH_ERROR_FORMAT("Error vkAcquireNextImageKHR - {}", string_VkResult(result));
			return;
		}
		assert(result == VkResult::VK_SUCCESS);

		// UI 커맨드 버퍼 설정
		if (cmd == nullptr)
		{
			cmd = context->GetCommandBufferPool().AllocateCommandBuffer(std::this_thread::get_id(), VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT);
			cmd->SetSignalSemaphores({ SignalSemaphore{ renderFinishedSemaphore } });
		}
		cmd->ResetCommand();

		// UI 커맨드 빌드
		cmd->Build(
			[&]
			{
				const VulkanFramebuffer* mainFramebuffer = static_cast<const VulkanFramebuffer*>(context->GetMainFramebuffer(imgIdx));
				auto& uiRenderPass = context->GetUIRenderPass();
				VulkanImageBuffer& swapchainImageBuffer = context->GetSwapChain().GetSwapChainImages()[imgIdx];

				if (uiRenderPass.GetInitialColorLayout() != VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED && swapchainImageBuffer.GetLayout() != uiRenderPass.GetInitialColorLayout())
					swapchainImageBuffer.ChangeLayoutCommand(cmd->GetCommandBuffer(), uiRenderPass.GetInitialColorLayout());
				auto msaaImg = mainFramebuffer->GetColorImg();
				if (msaaImg->GetLayout() == VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED)
					msaaImg->ChangeLayoutCommand(cmd->GetCommandBuffer(), VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
				if (uiRenderPass.GetInitialDepthLayout() != VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED && uiRenderPass.GetInitialDepthLayout() != mainFramebuffer->GetDepthImg()->GetLayout())
					mainFramebuffer->GetDepthImg()->ChangeLayoutCommand(cmd->GetCommandBuffer(), uiRenderPass.GetInitialDepthLayout());

				VkRenderPassBeginInfo renderPassInfo{};
				renderPassInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = uiRenderPass.GetVkRenderPass();
				renderPassInfo.framebuffer = mainFramebuffer->GetVkFramebuffer();
				renderPassInfo.renderArea.offset = { 0, 0 };
				renderPassInfo.renderArea.extent = context->GetSwapChain().GetSwapChainSize();

				std::array<VkClearValue, 3> clearMSAA;
				clearMSAA[0].color = { { 0.0f, 1.0f, 0.0f, 1.0f } };
				clearMSAA[1].color = { { 1.0f, 0.0f, 0.0f, 1.0f } };
				clearMSAA[2].depthStencil = { 1.0f, 0 };
				renderPassInfo.clearValueCount = static_cast<uint32_t>(clearMSAA.size());
				renderPassInfo.pClearValues = clearMSAA.data();

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

				swapchainImageBuffer.LayoutChangedByRenderPass(uiRenderPass.GetFinalColorLayout());
				mainFramebuffer->GetDepthImg()->LayoutChangedByRenderPass(uiRenderPass.GetFinalDepthLayout());
			},
			true
		);

		struct Command
		{
			RenderPipeline* pipeline;
			VulkanCommandBuffer* cmdBuffer;
		};
		std::vector<Command> recordedCommands;
		if (cameras.size() > 0)
		{
			// 카메라 데이터 GPU에 업로드
			std::vector<const Camera*> cams{};
			cams.reserve(cameras.size());
			for (auto camera : cameras)
			{
				if (!camera->GetActive())
					continue;
				camManager->UploadDataToGPU(*camera);
				cams.push_back(camera);
			}
			// 파이프라인 커맨드 기록
			std::vector<std::future<Command>> futureCommands;
			futureCommands.reserve(renderPipelines.size());
			for (auto& renderPipeline : renderPipelines)
			{
				futureCommands.push_back(core::ThreadPool::GetInstance()->AddTask(
					[&, pipeline = renderPipeline.get()]() -> Command
					{
						std::thread::id tid{ std::this_thread::get_id() };
						auto pipelineImpl = static_cast<VulkanRenderPipelineImpl*>(pipeline->GetImpl());
						auto pipelineCmd = context->GetCommandBufferPool().AllocateCommandBuffer(tid, VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT);
						assert(pipelineCmd != nullptr);
						pipelineImpl->SetCommandBuffer(*pipelineCmd);

						pipelineCmd->Build([&] { pipeline->RecordCommand(cams, imgIdx); }, true);
						return { pipeline, pipelineCmd };
					}
				));
			}
			for (auto& futureCommand : futureCommands)
				recordedCommands.push_back(futureCommand.get());
			
			// 타임라인 세마포어를 사용하여 순서대로 커맨드 제출
			for (const auto& recordedCmd : recordedCommands)
			{
				// 첫번째 파이프라인은 스왑체인 이미지를 기다리고 완료 시 세마포어 신호를 1로 바꾼다.
				if (timelineValue % recordedCommands.size() == 0)
				{
					uint64_t signalValue = timelineValue + 1; // 미리 계산
					timelineValueAtomic.store(signalValue, std::memory_order::memory_order_release);

					// 스왑체인 이미지가 준비 되기전까지 COLOR_ATTACHMENT단계에서 대기해야 함
					recordedCmd.cmdBuffer->SetWaitSemaphores({ 
						WaitSemaphore
						{ 
							imageAvailableSemaphore, 
							VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT 
						} 
					});
					recordedCmd.cmdBuffer->SetSignalSemaphores({ SignalSemaphore{ timelineSemaphore, true, ++timelineValue } });
				}
				else
				{
					recordedCmd.cmdBuffer->SetWaitSemaphores({ 
						WaitSemaphore
						{ 
							timelineSemaphore, 
							VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
							true, 
							timelineValue++
						} 
					});
					recordedCmd.cmdBuffer->SetSignalSemaphores({ SignalSemaphore{ timelineSemaphore, true, timelineValue } });
				}
				
				context->GetQueueManager().Submit(VulkanQueueManager::Role::Graphics, *recordedCmd.cmdBuffer, nullptr);
			}
			cmd->SetWaitSemaphores({ 
				WaitSemaphore
				{ 
					timelineSemaphore, 
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
					true, 
					timelineValue 
				} 
			});
		}
		else
		{
			// 카메라가 없는 경우는 UI 커맨드 혼자뿐.
			cmd->SetWaitSemaphores({ WaitSemaphore{ imageAvailableSemaphore, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT } });
		}
		context->GetQueueManager().Submit(VulkanQueueManager::Role::Graphics, *cmd, inFlightFence);

		uint32_t drawCallCount = 0;
		for (auto& renderPipeline : renderPipelines)
			drawCallCount += renderPipeline->GetDrawCallCount();
		SetDrawCallCount(drawCallCount);

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
		VkSwapchainKHR swapChains[] = { context->GetSwapChain().GetSwapChain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imgIdx;
		presentInfo.pResults = nullptr;

		vkQueuePresentKHR(context->GetQueueManager().GetQueue(VulkanQueueManager::Role::Present), &presentInfo);
		
		result = vkWaitForFences(context->GetDevice(), 1, &inFlightFence, true, std::numeric_limits<uint64_t>::max());
		if (result != VkResult::VK_SUCCESS)
		{
			SH_ERROR_FORMAT("Error vkWaitForFences: {}", string_VkResult(result));
		}
		vkResetFences(context->GetDevice(), 1, &inFlightFence);
		for (auto& cmd : recordedCommands)
			context->GetCommandBufferPool().DeallocateCommandBuffer(*cmd.cmdBuffer);
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
	SH_RENDER_API auto VulkanRenderer::GetTimelineSemaphore() const -> VkSemaphore
	{
		return timelineSemaphore;
	}
	SH_RENDER_API auto VulkanRenderer::GetTimelineValue() const -> uint64_t
	{
		return timelineValueAtomic.load(std::memory_order::memory_order_acquire);
	}
	SH_RENDER_API auto VulkanRenderer::GetContext() const -> IRenderContext*
	{
		return context.get();
	}
	SH_RENDER_API auto VulkanRenderer::GetCommandBuffer() const -> VulkanCommandBuffer*
	{
		return cmd;
	}
	SH_RENDER_API void VulkanRenderer::Sync()
	{
		Renderer::Sync();
	}
}//namespace

