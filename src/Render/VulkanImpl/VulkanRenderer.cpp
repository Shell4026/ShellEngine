#include "VulkanRenderer.h"
#include "VulkanContext.h"
#include "VulkanSwapChain.h"
#include "VulkanQueueManager.h"
#include "VulkanPipelineManager.h"
#include "VulkanCommandBufferPool.h"
#include "Drawable.h"

#include "Core/Util.h"
#include "Core/Logger.h"

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
			DestroySyncObjects();
		}
	}

	SH_RENDER_API void VulkanRenderer::Clear()
	{
		vkDeviceWaitIdle(context->GetDevice());
		Renderer::Clear();
		frameFences.clear();
	}

	void VulkanRenderer::CreateContext(const window::Window& win)
	{
		context = std::make_unique<VulkanContext>(win);
		context->Init();
	}
	SH_RENDER_API void VulkanRenderer::DestroyContext()
	{
		context->Clear();
		context.reset();
	}
	SH_RENDER_API bool VulkanRenderer::Init(sh::window::Window& win)
	{
		Renderer::Init(win);

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

		if (renderer != nullptr)
			IRenderThrMethod<ScriptableRenderer>::ResetSwapChainStates(*renderer);

		return context->ReSizing();
	}

	SH_RENDER_API void VulkanRenderer::WaitForCurrentFrame()
	{
		vkDeviceWaitIdle(context->GetDevice());
	}

	SH_RENDER_API void VulkanRenderer::Render()
	{
		using SignalSemaphore = VulkanCommandBuffer::SignalSemaphore;
		using WaitSemaphore = VulkanCommandBuffer::WaitSemaphore;

		if (!isInit || bPause.load(std::memory_order::memory_order_acquire))
			return;

		Renderer::Render();
		if (renderer == nullptr)
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
		core::ArrayView<RenderData> renderDatas = IRenderThrMethod<RenderDataManager>::GetRenderDatas(context->GetRenderDataManager());
		std::size_t viewIdx = 0;
		for (RenderData& rd : renderDatas)
		{
			rd.frameIndex = imgIdx;
			rd.drawables = &drawables;

			IRenderThrMethod<ScriptableRenderer>::Setup(*renderer, rd);
			IRenderThrMethod<ScriptableRenderer>::Execute(*renderer, rd); // ScriptableRenderer에 등록된 패스들을 렌더큐 순서대로, 병렬적으로 커맨드에 기록
		}

		IRenderThrMethod<ScriptableRenderer>::ExecuteTransfer(*renderer, imgIdx);

		const std::vector<ScriptableRenderer::RecordedCommand>& recordedCmds = renderer->GetRecordedCommands();
		if (recordedCmds.size() == 1)
		{
			VulkanCommandBuffer& cmd = static_cast<VulkanCommandBuffer&>(recordedCmds.front().cmd);
			cmd.AddWaitSemaphore(
				WaitSemaphore
				{
					imageAvailableSemaphore,
					VkPipelineStageFlagBits::VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
				}
			);
			cmd.AddSignalSemaphore(SignalSemaphore{ renderFinishedSemaphore });
			context->GetQueueManager().Submit(VulkanQueueManager::Role::Graphics, cmd, inFlightFence);
		}
		else
		{
			VulkanCommandBuffer& endCmd = static_cast<VulkanCommandBuffer&>(recordedCmds.back().cmd);
			endCmd.AddSignalSemaphore(SignalSemaphore{ renderFinishedSemaphore });

			bool bUsedImageAvailableSemaphore = false;
			for (const ScriptableRenderer::RecordedCommand& recordedCmd : recordedCmds)
			{
				ScriptableRenderPass& pass = recordedCmd.pass;
				VulkanCommandBuffer& cmd = static_cast<VulkanCommandBuffer&>(recordedCmd.cmd);

				// 스왑체인을 쓰는 첫 패스는 imageAvailableSemaphore대기
				if (!bUsedImageAvailableSemaphore)
				{
					const auto& rts = pass.GetRenderTextures();
					if (rts.find(nullptr) != rts.end())
					{
						cmd.AddWaitSemaphore(
							WaitSemaphore
							{
								imageAvailableSemaphore,
								VkPipelineStageFlagBits::VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
							}
						);
						bUsedImageAvailableSemaphore = true;
					}
				}

				VkFence fence = nullptr;
				if (&cmd == &endCmd)
					fence = inFlightFence;
				context->GetQueueManager().Submit(VulkanQueueManager::Role::Graphics, cmd, fence);
			}
		}

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

		IRenderThrMethod<ScriptableRenderer>::CallReadbacks(*renderer);

		IRenderThrMethod<ScriptableRenderer>::ResetSubmittedCommands(*renderer, *context);
	}

	SH_RENDER_API auto VulkanRenderer::GetWidth() const -> uint32_t
	{
		return context->GetSwapChain().GetSwapChainSize().width;
	}
	SH_RENDER_API auto VulkanRenderer::GetHeight() const -> uint32_t
	{
		return context->GetSwapChain().GetSwapChainSize().height;
	}
	SH_RENDER_API auto VulkanRenderer::GetContext() const -> IRenderContext*
	{
		return context.get();
	}
	SH_RENDER_API void VulkanRenderer::Sync()
	{
		Renderer::Sync();
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
}//namespace

