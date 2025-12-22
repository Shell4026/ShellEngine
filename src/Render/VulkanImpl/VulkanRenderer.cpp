#include "VulkanRenderer.h"
#include "VulkanContext.h"
#include "VulkanSwapChain.h"
#include "VulkanQueueManager.h"
#include "VulkanPipelineManager.h"
#include "VulkanCameraBuffers.h"
#include "VulkanCommandBufferPool.h"
#include "Drawable.h"

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
			context->Clear();
		}
	}

	SH_RENDER_API void VulkanRenderer::Clear()
	{
		vkDeviceWaitIdle(context->GetDevice());

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

		if (renderer != nullptr)
			IRenderThrMethod<ScriptableRenderer>::ResetSwapChainStates(*renderer);

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
		if (renderer == nullptr)
			return;
		if (cameras.size() == 0)
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

		for (const Camera* cam : cams)
		{
			std::vector<Drawable*> filteredDrawables;

			RenderTarget rt{};
			rt.frameIndex = imgIdx;
			rt.camera = cam;
			rt.target = cam->GetRenderTexture();
			rt.drawables = &filteredDrawables;
			
			for (Drawable* drawable : drawables)
			{
				if (cam->CheckRenderTag(drawable->GetRenderTagId()))
					filteredDrawables.push_back(drawable);
			}
			IRenderThrMethod<ScriptableRenderer>::Setup(*renderer, rt);
			IRenderThrMethod<ScriptableRenderer>::Execute(*renderer, rt); // ScriptableRenderer에 등록된 패스들을 렌더큐 순서대로, 병렬적으로 커맨드에 기록
		}
		IRenderThrMethod<ScriptableRenderer>::ExecuteTransfer(*renderer, imgIdx);

		const auto& submittedCmds = renderer->GetSubmittedCommands();
		if (submittedCmds.size() == 1)
		{
			VulkanCommandBuffer& cmd = static_cast<VulkanCommandBuffer&>(submittedCmds.front().cmd);
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
			VulkanCommandBuffer& endCmd = static_cast<VulkanCommandBuffer&>(submittedCmds.back().cmd);
			endCmd.AddSignalSemaphore(SignalSemaphore{ renderFinishedSemaphore });

			bool bUsedImageAvailableSemaphore = false;
			for (const auto& submittedCmd : submittedCmds)
			{
				ScriptableRenderPass& pass = submittedCmd.pass;
				VulkanCommandBuffer& cmd = static_cast<VulkanCommandBuffer&>(submittedCmd.cmd);

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
		uint32_t drawCallCount = 0;
		//for (auto& renderPipeline : renderPipelines)
		//	drawCallCount += renderPipeline->GetDrawCallCount();
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

		IRenderThrMethod<ScriptableRenderer>::CallReadbacks(*renderer);

		IRenderThrMethod<ScriptableRenderer>::ResetSubmittedCommands(*renderer, *context);
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
	SH_RENDER_API void VulkanRenderer::Sync()
	{
		Renderer::Sync();
	}
}//namespace

