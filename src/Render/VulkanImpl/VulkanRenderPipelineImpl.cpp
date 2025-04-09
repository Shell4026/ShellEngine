#include "VulkanRenderPipelineImpl.h"
#include "VulkanContext.h"
#include "VulkanFramebuffer.h"
#include "VulkanSwapChain.h"
#include "VulkanPipelineManager.h"
#include "VulkanVertexBuffer.h"
#include "VulkanShaderPass.h"
#include "VulkanRenderPass.h"
#include "VulkanUniformBuffer.h"
#include "VulkanCameraBuffers.h"
#include "VulkanRenderPassManager.h"
#include "RenderTexture.h"
#include "Camera.h"
#include "Mesh.h"
#include "Material.h"

#include "Core/Util.h"

#include <cassert>

namespace sh::render::vk
{
	VulkanRenderPipelineImpl::VulkanRenderPipelineImpl(VulkanContext& context) :
		context(context)
	{
		cmd = context.GetCommandBuffer(core::ThreadType::Render);

		cameraManager = VulkanCameraBuffers::GetInstance();
	}

	SH_RENDER_API void VulkanRenderPipelineImpl::RenderDrawable(const core::Name& lightingPassName, const Camera& camera, const std::vector<RenderGroup>& renderGroups, VkRenderPass renderPass)
	{
		assert(cmd);

		uint32_t cameraOffset = cameraManager->GetDynamicOffset(camera);

		for (auto& renderGroup : renderGroups)
		{
			const Material* mat = renderGroup.material;
			assert(mat);

			auto passVectorPtr = renderGroup.material->GetShader()->GetShaderPasses(lightingPassName);
			if (passVectorPtr == nullptr)
				continue;
			for (auto& pass : *passVectorPtr)
			{
				auto pipelineHandle = context.GetPipelineManager().
					GetOrCreatePipelineHandle(renderPass, static_cast<VulkanShaderPass&>(*pass.get()), renderGroup.topology);
				context.GetPipelineManager().BindPipeline(cmd->GetCommandBuffer(), pipelineHandle);
				VkPipelineLayout layout = static_cast<VulkanShaderPass&>(*pass).GetPipelineLayout();
				uint32_t setSize = static_cast<VulkanShaderPass*>(pass.get())->GetSetCount();

				if (setSize > 0)
				{
					auto cameraUniformBuffer = static_cast<VulkanUniformBuffer*>(mat->GetMaterialData().GetUniformBuffer(*pass.get(),
						UniformStructLayout::Type::Camera, core::ThreadType::Render));

					VkDescriptorSet cameraDescriptorSet = VK_NULL_HANDLE;
					uint32_t dynamicCount = 0;
					if (cameraUniformBuffer)
					{
						cameraDescriptorSet = cameraUniformBuffer->GetVkDescriptorSet();
						dynamicCount = 1;
					}
					else
						cameraDescriptorSet = context.GetEmptyDescriptorSet();

					vkCmdBindDescriptorSets(cmd->GetCommandBuffer(),
						VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, layout, static_cast<uint32_t>(UniformStructLayout::Type::Camera), 1,
						&cameraDescriptorSet, dynamicCount, &cameraOffset);
				}
				if (setSize > 2)
				{
					auto materialUniformBuffer = static_cast<VulkanUniformBuffer*>(mat->GetMaterialData().GetUniformBuffer(*pass.get(),
						UniformStructLayout::Type::Material, core::ThreadType::Render));

					VkDescriptorSet materialDescriptorSet = VK_NULL_HANDLE;
					if (materialUniformBuffer)
						materialDescriptorSet = materialUniformBuffer->GetVkDescriptorSet();
					else
						materialDescriptorSet = context.GetEmptyDescriptorSet();

					vkCmdBindDescriptorSets(cmd->GetCommandBuffer(),
						VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, layout, static_cast<uint32_t>(UniformStructLayout::Type::Material), 1,
						&materialDescriptorSet, 0, nullptr);
				}

				for (auto& drawable : renderGroup.drawables)
				{
					if (!camera.CheckRenderTag(drawable->GetRenderTagId()))
						continue;

					const Mesh* mesh = drawable->GetMesh();
					mesh->GetVertexBuffer()->Bind();

					if (setSize > 1)
					{
						auto objectUniformBuffer = static_cast<VulkanUniformBuffer*>(drawable->GetMaterialData().GetUniformBuffer(*pass.get(),
							UniformStructLayout::Type::Object, core::ThreadType::Render));
						
						if (pass->IsUsingLight())
							drawable->GetMaterialData().SetUniformData(*pass, UniformStructLayout::Type::Object, 0, &drawable->GetLightData(), core::ThreadType::Render);

						VkDescriptorSet objectDescriptorSet = VK_NULL_HANDLE;
						if (objectUniformBuffer)
							objectDescriptorSet = objectUniformBuffer->GetVkDescriptorSet();
						else
							objectDescriptorSet = context.GetEmptyDescriptorSet();

						

						vkCmdBindDescriptorSets(cmd->GetCommandBuffer(),
							VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, layout, static_cast<uint32_t>(UniformStructLayout::Type::Object), 1,
							&objectDescriptorSet, 0, nullptr);
					}
					if (pass->HasConstantUniform())
						vkCmdPushConstants(cmd->GetCommandBuffer(), layout, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &drawable->GetModelMatrix());

					vkCmdDrawIndexed(cmd->GetCommandBuffer(), mesh->GetIndices().size(), 1, 0, 0, 0);
					++drawCall;
				}
			}
		}
	}
	SH_RENDER_API void VulkanRenderPipelineImpl::RecordCommand(const core::Name& lightingPassName, const std::vector<const Camera*>& cameras, const std::vector<RenderGroup>& renderData, uint32_t imgIdx)
	{
		drawCall = 0;

		assert(cmd != nullptr);
		std::array<VkClearValue, 2> clear;
		clear[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
		clear[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.clearValueCount = bClearFramebuffer ? static_cast<uint32_t>(clear.size()) : 0;
		renderPassInfo.pClearValues = bClearFramebuffer ? clear.data() : nullptr;

		VkViewport viewport{};
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };

		VkCommandBuffer commandBuffer = cmd->GetCommandBuffer();
		for (auto camera : cameras)
		{
			RenderTexture* renderTexture = camera->GetRenderTexture();

			VkRenderPass renderPass = VK_NULL_HANDLE;
			if (renderTexture == nullptr)
			{
				const VulkanFramebuffer* mainFramebuffer = static_cast<const VulkanFramebuffer*>(context.GetMainFramebuffer(imgIdx));
				renderPass = mainFramebuffer->GetRenderPass()->GetVkRenderPass();
				renderPassInfo.renderPass = renderPass;
				renderPassInfo.framebuffer = mainFramebuffer->GetVkFramebuffer();
				renderPassInfo.renderArea.extent = context.GetSwapChain().GetSwapChainSize();

				float width = context.GetViewportEnd().x - context.GetViewportStart().x;
				float height = context.GetViewportEnd().y - context.GetViewportStart().y;
				float surfWidth = static_cast<float>(context.GetSwapChain().GetSwapChainSize().width);
				float surfHeight = static_cast<float>(context.GetSwapChain().GetSwapChainSize().height);
				viewport.x = context.GetViewportStart().x;
				viewport.y = context.GetViewportEnd().y;
				viewport.width = std::min(width, surfWidth);
				viewport.height = -std::min(height, surfHeight);

				scissor.extent = context.GetSwapChain().GetSwapChainSize();
			}
			else
			{
				auto vkFramebuffer = static_cast<VulkanFramebuffer*>(renderTexture->GetFramebuffer(core::ThreadType::Render));
				auto config = vkFramebuffer->GetRenderPass()->GetConfig();
				if (config.bClear != bClearFramebuffer)
				{
					config.bClear = bClearFramebuffer;
					renderPass = context.GetRenderPassManager().GetOrCreateRenderPass(config).GetVkRenderPass();
				}
				else
					renderPass = vkFramebuffer->GetRenderPass()->GetVkRenderPass();

				renderPassInfo.renderPass = renderPass;
				renderPassInfo.framebuffer = vkFramebuffer->GetVkFramebuffer();
				renderPassInfo.renderArea.extent = { vkFramebuffer->GetWidth(), vkFramebuffer->GetHeight() };

				viewport.x = 0;
				viewport.y = vkFramebuffer->GetHeight();
				viewport.width = vkFramebuffer->GetWidth();
				viewport.height = -static_cast<float>(vkFramebuffer->GetHeight());

				scissor.extent = { vkFramebuffer->GetWidth(), vkFramebuffer->GetHeight() };

				renderTexture->SetDirty();
			}
			
			vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

			RenderDrawable(lightingPassName, *camera, renderData, renderPass);

			vkCmdEndRenderPass(commandBuffer);
		}
	}

	SH_RENDER_API void VulkanRenderPipelineImpl::SetClear(bool bClear)
	{
		bClearFramebuffer = bClear;
	}

	SH_RENDER_API auto VulkanRenderPipelineImpl::GetDrawCallCount() const -> uint32_t
	{
		return drawCall;
	}
}//namespace