#include "pch.h"
#include "VulkanRenderer.h"
#include "VulkanContext.h"
#include "VulkanSwapChain.h"
#include "VulkanDrawable.h"
#include "VulkanShader.h"
#include "VulkanQueueManager.h"
#include "VulkanPipelineManager.h"
#include "VulkanFramebuffer.h"
#include "Mesh.h"
#include "Material.h"

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
			Clean();
	}

	SH_RENDER_API void VulkanRenderer::Clean()
	{
		Renderer::Clean();

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

	SH_RENDER_API bool VulkanRenderer::Init(const sh::window::Window& win)
	{
		Renderer::Init(win);

		context = std::make_unique<VulkanContext>(win);
		context->Init();

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

		return context->ReSizing();
	}

	inline void VulkanRenderer::RenderDrawable(IDrawable* iDrawable, VkCommandBuffer cmd)
	{
		if (!core::IsValid(iDrawable))
			return;
		VulkanDrawable* drawable = static_cast<VulkanDrawable*>(iDrawable);
		const Mesh* mesh = drawable->GetMesh();
		const Material* mat = drawable->GetMaterial();

		assert(mesh);
		assert(mat);
		if (!sh::core::IsValid(mesh) || !sh::core::IsValid(mat))
			return;

		VulkanShader* shader = static_cast<VulkanShader*>(mat->GetShader());
		if (!sh::core::IsValid(shader))
			return;

		if (!context->GetPipelineManager().BindPipeline(cmd, drawable->GetPipelineHandle()))
			return;

		mesh->GetVertexBuffer()->Bind();

		VkDescriptorSet localDescSet = drawable->GetDescriptorSet(core::ThreadType::Render);
		VkDescriptorSet descSet = static_cast<VulkanUniformBuffer*>(mat->GetUniformBuffer(core::ThreadType::Render))->GetVkDescriptorSet();
		std::array<VkDescriptorSet, 2> descriptorSets = { localDescSet, descSet };
		
		assert(localDescSet);
		assert(descSet);

		vkCmdBindDescriptorSets(cmd,
			VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS,
			shader->GetPipelineLayout(), 0, descriptorSets.size(),
			descriptorSets.data(), 0, nullptr);

		vkCmdDrawIndexed(cmd, mesh->GetIndices().size(), 1, 0, 0, 0);
	}

	SH_RENDER_API bool VulkanRenderer::IsInit() const
	{
		return isInit;
	}

	SH_RENDER_API void VulkanRenderer::WaitForCurrentFrame()
	{
		vkWaitForFences(context->GetDevice(), 1, &inFlightFence, VK_TRUE, UINT64_MAX);
	}

	SH_RENDER_API void VulkanRenderer::Render(float deltaTime)
	{
		if (!isInit || bPause.load(std::memory_order::memory_order_acquire))
			return;
		if (drawList[core::ThreadType::Render].empty())
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
		cmd->SetWaitSemaphore({ imageAvailableSemaphore });
		cmd->SetSignalSemaphore({ renderFinishedSemaphore });
		cmd->SetWaitStage({
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		});

		if (drawList[core::ThreadType::Render].empty())
			return;

		VkCommandBuffer cmdBuffer = cmd->GetCommandBuffer();

		cmd->Build([&]()
		{
			bool mainPassProcessed = false;

			for (auto& [camera, drawables] : drawList[core::ThreadType::Render])
			{
				auto renderTexture = camera->GetRenderTexture();
				// 프레임버퍼에 그림
				if(renderTexture != nullptr)
				{
					auto framebuffer = static_cast<const VulkanFramebuffer*>(renderTexture->GetFramebuffer(core::ThreadType::Render));
					std::array<VkClearValue, 2> clear;
					clear[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
					clear[1].depthStencil = { 1.0f, 0 };

					VkRenderPassBeginInfo renderPassInfo{};
					renderPassInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
					renderPassInfo.renderPass = framebuffer->GetRenderPass();
					renderPassInfo.framebuffer = framebuffer->GetVkFramebuffer();
					renderPassInfo.renderArea.offset = { 0, 0 };
					renderPassInfo.renderArea.extent = { framebuffer->GetWidth(), framebuffer->GetHeight() };
					renderPassInfo.clearValueCount = static_cast<uint32_t>(clear.size());
					renderPassInfo.pClearValues = clear.data();

					//Begin RenderPass
					vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

					VkViewport viewport{};
					viewport.x = 0;
					viewport.y = framebuffer->GetHeight();
					viewport.width = framebuffer->GetWidth();
					viewport.height = -static_cast<float>(framebuffer->GetHeight());
					viewport.minDepth = 0.0f;
					viewport.maxDepth = 1.0f;
					vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

					VkRect2D scissor{};
					scissor.offset = { 0, 0 };
					scissor.extent = { framebuffer->GetWidth(), framebuffer->GetHeight() };
					vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

					for (auto iDrawable : drawables)
						RenderDrawable(iDrawable, cmdBuffer);

					//End RenderPass
					vkCmdEndRenderPass(cmdBuffer);

					renderTexture->SetDirty();
				}
				//MainPass
				else
				{
					mainPassProcessed = true;

					const VulkanFramebuffer* mainFramebuffer = static_cast<const VulkanFramebuffer*>(context->GetMainFramebuffer(imgIdx));

					VkRenderPassBeginInfo renderPassInfo{};
					std::array<VkClearValue, 2> clear;
					clear[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
					clear[1].depthStencil = { 1.0f, 0 };

					renderPassInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
					renderPassInfo.renderPass = mainFramebuffer->GetRenderPass();
					renderPassInfo.framebuffer = mainFramebuffer->GetVkFramebuffer();
					renderPassInfo.renderArea.offset = { 0, 0 };
					renderPassInfo.renderArea.extent = context->GetSwapChain().GetSwapChainSize();
					renderPassInfo.clearValueCount = static_cast<uint32_t>(clear.size());
					renderPassInfo.pClearValues = clear.data();
					vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

					VkViewport viewport{};
					float width = viewportEnd.x - viewportStart.x;
					float height = viewportEnd.y - viewportStart.y;
					float surfWidth = static_cast<float>(context->GetSwapChain().GetSwapChainSize().width);
					float surfHeight = static_cast<float>(context->GetSwapChain().GetSwapChainSize().height);
					viewport.x = viewportStart.x;
					viewport.y = viewportEnd.y;
					viewport.width = std::min(width, surfWidth);
					viewport.height = -std::min(height, surfHeight);
					viewport.minDepth = 0.0f;
					viewport.maxDepth = 1.0f;
					vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

					VkRect2D scissor{};
					scissor.offset = { 0, 0 };
					scissor.extent = context->GetSwapChain().GetSwapChainSize();
					vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);

					for (auto& func : drawCalls)
						func();

					vkCmdEndRenderPass(cmdBuffer);
				}
			}//for (auto& [camera, drawables] : drawList[RENDER_THREAD])
			// 모든 카메라에 프레임 버퍼가 존재하는 특수한 경우
			if (mainPassProcessed == false)
			{
				const VulkanFramebuffer* mainFramebuffer = static_cast<const VulkanFramebuffer*>(context->GetMainFramebuffer(imgIdx));

				VkRenderPassBeginInfo renderPassInfo{};
				std::array<VkClearValue, 2> clear;
				clear[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
				clear[1].depthStencil = { 1.0f, 0 };

				renderPassInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = mainFramebuffer->GetRenderPass();
				renderPassInfo.framebuffer = mainFramebuffer->GetVkFramebuffer();
				renderPassInfo.renderArea.offset = { 0, 0 };
				renderPassInfo.renderArea.extent = context->GetSwapChain().GetSwapChainSize();
				renderPassInfo.clearValueCount = static_cast<uint32_t>(clear.size());
				renderPassInfo.pClearValues = clear.data();
				vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

				VkViewport viewport{};
				float width = viewportEnd.x - viewportStart.x;
				float height = viewportEnd.y - viewportStart.y;
				float surfWidth = static_cast<float>(context->GetSwapChain().GetSwapChainSize().width);
				float surfHeight = static_cast<float>(context->GetSwapChain().GetSwapChainSize().height);
				viewport.x = viewportStart.x;
				viewport.y = viewportEnd.y;
				viewport.width = std::min(width, surfWidth);
				viewport.height = -std::min(height, surfHeight);
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;
				vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

				for (auto& func : drawCalls)
					func();

				vkCmdEndRenderPass(cmdBuffer);
			}
		}, nullptr);

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
}//namespace

