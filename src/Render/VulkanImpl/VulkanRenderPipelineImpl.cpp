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
#include "VulkanCommandBuffer.h"
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
		cameraManager = VulkanCameraBuffers::GetInstance();
	}
	VulkanRenderPipelineImpl::~VulkanRenderPipelineImpl()
	{
	}
	SH_RENDER_API void VulkanRenderPipelineImpl::RenderDrawable(const core::Name& lightingPassName, const Camera& camera, const std::vector<RenderGroup>& renderGroups, const VulkanRenderPass& renderPass)
	{
		uint32_t cameraOffset = cameraManager->GetDynamicOffset(camera);

		// 렌더 그룹은 메테리얼별로 나눠져있음
		for (const RenderGroup& renderGroup : renderGroups)
		{
			const Material* mat = renderGroup.material;
			assert(mat);

			auto passVectorPtr = renderGroup.material->GetShader()->GetShaderPasses(lightingPassName);
			if (passVectorPtr == nullptr)
				continue;
			for (ShaderPass& pass : *passVectorPtr)
			{
				if (pass.IsPendingKill())
					continue;

				auto pipelineHandle = context.GetPipelineManager().
					GetOrCreatePipelineHandle(renderPass, static_cast<VulkanShaderPass&>(pass), renderGroup.topology);
				context.GetPipelineManager().BindPipeline(cmd->GetCommandBuffer(), pipelineHandle);
				VkPipelineLayout layout = static_cast<VulkanShaderPass&>(pass).GetPipelineLayout();
				uint32_t setSize = static_cast<VulkanShaderPass&>(pass).GetSetCount();

				// set = 0 카메라
				// set = 1 객체 고유
				// set = 2 메테리얼
				if (setSize > 0)
					BindCameraSet(layout, pass, *mat, cameraOffset);
				if (setSize > 2)
					BindMaterialSet(layout, pass, *mat);

				for (auto drawable : renderGroup.drawables)
				{
					if (drawable == nullptr || !camera.CheckRenderTag(drawable->GetRenderTagId()))
						continue;
					const Mesh* mesh = drawable->GetMesh();
					if (!core::IsValid(mesh))
						continue;

					if (setSize > 1)
						BindObjectSet(layout, pass, *drawable);

					if (pass.HasConstantUniform())
						vkCmdPushConstants(cmd->GetCommandBuffer(), layout,
							VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT,
							0, sizeof(glm::mat4),
							&drawable->GetModelMatrix(core::ThreadType::Render));

					DrawMesh(pass, *mesh);
					++drawCall;
				}
			}
		}
	}
	SH_RENDER_API void VulkanRenderPipelineImpl::RecordCommand(const core::Name& lightingPassName, const Camera& camera, const std::vector<RenderGroup>& renderData, uint32_t imgIdx)
	{
		drawCall = 0;

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderArea.offset = { 0, 0 };

		VkViewport viewport{};
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };

		VkCommandBuffer commandBuffer = cmd->GetCommandBuffer();

		RenderTexture* renderTexture = camera.GetRenderTexture();

		const VulkanRenderPass* renderPass = nullptr;
		if (!core::IsValid(renderTexture))
		{
			const VulkanFramebuffer* mainFramebuffer = static_cast<const VulkanFramebuffer*>(context.GetMainFramebuffer(imgIdx));
			VulkanRenderPass::Config config{ mainFramebuffer->GetRenderPass()->GetConfig() };
			if (config.bClear != bClearFramebuffer)
			{
				config.bClear = bClearFramebuffer;
				renderPass = &context.GetRenderPassManager().GetOrCreateRenderPass(config);
			}
			else
				renderPass = mainFramebuffer->GetRenderPass();

			SetClearSetting(renderPassInfo, context.GetSampleCount() != VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT);

			renderPassInfo.renderPass = renderPass->GetVkRenderPass();
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
			if (mainFramebuffer->GetColorImg() != nullptr)
				mainFramebuffer->GetColorImg()->LayoutChangedByRenderPass(renderPass->GetFinalColorLayout());
		}
		else
		{
			auto vkFramebuffer = static_cast<VulkanFramebuffer*>(renderTexture->GetFramebuffer());
			VulkanRenderPass::Config config{ vkFramebuffer->GetRenderPass()->GetConfig() };
			if (config.bClear != bClearFramebuffer)
			{
				config.bClear = bClearFramebuffer;
				renderPass = &context.GetRenderPassManager().GetOrCreateRenderPass(config);
			}
			else
				renderPass = vkFramebuffer->GetRenderPass();
			SetClearSetting(renderPassInfo, config.sampleCount != VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT);

			renderPassInfo.renderPass = renderPass->GetVkRenderPass();
			renderPassInfo.framebuffer = vkFramebuffer->GetVkFramebuffer();
			renderPassInfo.renderArea.extent = { vkFramebuffer->GetWidth(), vkFramebuffer->GetHeight() };

			viewport.x = 0;
			viewport.y = vkFramebuffer->GetHeight();
			viewport.width = vkFramebuffer->GetWidth();
			viewport.height = -static_cast<float>(vkFramebuffer->GetHeight());

			scissor.extent = { vkFramebuffer->GetWidth(), vkFramebuffer->GetHeight() };
			vkFramebuffer->GetColorImg()->LayoutChangedByRenderPass(renderPass->GetFinalColorLayout());
		}

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		RenderDrawable(lightingPassName, camera, renderData, *renderPass);

		vkCmdEndRenderPass(commandBuffer);
	}

	SH_RENDER_API void VulkanRenderPipelineImpl::SetClear(bool bClear)
	{
		bClearFramebuffer = bClear;
	}

	SH_RENDER_API void VulkanRenderPipelineImpl::SetCommandBuffer(VulkanCommandBuffer& cmd)
	{
		this->cmd = &cmd;
	}

	SH_RENDER_API auto VulkanRenderPipelineImpl::GetDrawCallCount() const -> uint32_t
	{
		return drawCall;
	}
	SH_RENDER_API auto VulkanRenderPipelineImpl::GetCommandBuffer() const -> VulkanCommandBuffer*
	{
		return cmd;
	}
	void VulkanRenderPipelineImpl::SetClearSetting(VkRenderPassBeginInfo& beginInfo, bool bMSAA)
	{
		static std::array<VkClearValue, 2> clear;
		clear[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
		clear[1].depthStencil = { 1.0f, 0 };

		static std::array<VkClearValue, 3> clearMSAA;
		clearMSAA[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } }; // sample 이미지
		clearMSAA[1].color = { { 0.0f, 0.0f, 0.0f, 1.0f } }; // resolve 이미지
		clearMSAA[2].depthStencil = { 1.0f, 0 };

		beginInfo.clearValueCount = bMSAA ? static_cast<uint32_t>(clearMSAA.size()) : static_cast<uint32_t>(clear.size());
		beginInfo.pClearValues = bMSAA ? clearMSAA.data() : clear.data();
	}
	void VulkanRenderPipelineImpl::BindCameraSet(VkPipelineLayout layout, const ShaderPass& pass, const Material& mat, uint32_t cameraOffset)
	{
		// 카메라 데이터는 다이나믹 디스크립터셋
		auto cameraUBO = static_cast<VulkanUniformBuffer*>(
			mat.GetMaterialData().GetUniformBuffer(pass, UniformStructLayout::Type::Camera));

		VkDescriptorSet cameraSet = cameraUBO ? cameraUBO->GetVkDescriptorSet() : context.GetEmptyDescriptorSet();
		uint32_t dynamicCount = cameraUBO ? 1 : 0;

		vkCmdBindDescriptorSets(cmd->GetCommandBuffer(),
			VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, layout,
			static_cast<uint32_t>(UniformStructLayout::Type::Camera),
			1, &cameraSet, dynamicCount, &cameraOffset);
	}
	void VulkanRenderPipelineImpl::BindMaterialSet(VkPipelineLayout layout, const ShaderPass& pass, const Material& mat)
	{
		auto materialUniformBuffer = static_cast<VulkanUniformBuffer*>(
			mat.GetMaterialData().GetUniformBuffer(pass, UniformStructLayout::Type::Material));

		VkDescriptorSet materialDescriptorSet = materialUniformBuffer ? materialUniformBuffer->GetVkDescriptorSet() : context.GetEmptyDescriptorSet();

		vkCmdBindDescriptorSets(cmd->GetCommandBuffer(),
			VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 
			static_cast<uint32_t>(UniformStructLayout::Type::Material), 
			1, &materialDescriptorSet, 0, nullptr);
	}
	void VulkanRenderPipelineImpl::BindObjectSet(VkPipelineLayout layout, const ShaderPass& pass, Drawable& drawable)
	{
		auto objectUniformBuffer = static_cast<VulkanUniformBuffer*>(
			drawable.GetMaterialData().GetUniformBuffer(pass, UniformStructLayout::Type::Object));

		VkDescriptorSet objectDescriptorSet = objectUniformBuffer ? objectUniformBuffer->GetVkDescriptorSet() : context.GetEmptyDescriptorSet();

		vkCmdBindDescriptorSets(cmd->GetCommandBuffer(), 
			VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 
			static_cast<uint32_t>(UniformStructLayout::Type::Object), 
			1, &objectDescriptorSet, 0, nullptr);
	}

	void VulkanRenderPipelineImpl::DrawMesh(const ShaderPass& pass, const Mesh& mesh)
	{
		const VulkanVertexBuffer* vkVertexBuffer = static_cast<VulkanVertexBuffer*>(mesh.GetVertexBuffer());

		std::array<VkBuffer, 1> buffers = { vkVertexBuffer->GetVertexBuffer().GetBuffer() };

		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(cmd->GetCommandBuffer(), 0, 1, buffers.data(), offsets);
		vkCmdBindIndexBuffer(cmd->GetCommandBuffer(), vkVertexBuffer->GetIndexBuffer().GetBuffer(), 0, VkIndexType::VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cmd->GetCommandBuffer(), static_cast<uint32_t>(mesh.GetIndices().size()), 1, 0, 0, 0);
	}

}//namespace