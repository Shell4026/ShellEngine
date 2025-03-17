#include "VulkanBasePass.h"
#include "VulkanContext.h"
#include "VulkanFramebuffer.h"
#include "VulkanSwapChain.h"
#include "VulkanPipelineManager.h"
#include "VulkanVertexBuffer.h"
#include "VulkanShaderPass.h"
#include "VulkanRenderPass.h"
#include "VulkanUniformBuffer.h"
#include "VulkanCameraBuffers.h"
#include "RenderTexture.h"
#include "Camera.h"
#include "Mesh.h"
#include "Material.h"

#include "Core/Util.h"

#include <cassert>

namespace sh::render::vk
{
	SH_RENDER_API void VulkanBasePass::RenderDrawable(const Camera& camera, VkRenderPass renderPass, Drawable* drawable)
	{
		assert(cmd != nullptr);
		if (!core::IsValid(drawable) || cmd == nullptr)
			return;
		const Mesh* mesh = drawable->GetMesh();
		const Material* mat = drawable->GetMaterial();

		assert(mesh);
		assert(mat);
		if (!sh::core::IsValid(mesh) || !sh::core::IsValid(mat))
			return;

		Shader* shader = mat->GetShader();
		if (!sh::core::IsValid(shader))
			return;

		auto passVectorPtr = shader->GetShaderPasses(passName);
		if (passVectorPtr == nullptr)
			return;

		for (auto& pass : *passVectorPtr)
		{
			auto pipelineHandle = context->GetPipelineManager().
				GetOrCreatePipelineHandle(renderPass, static_cast<VulkanShaderPass&>(*pass.get()), mesh->GetTopology());

			if (!context->GetPipelineManager().BindPipeline(cmd->GetCommandBuffer(), pipelineHandle))
				continue;

			mesh->GetVertexBuffer()->Bind();

			uint32_t setSize = static_cast<VulkanShaderPass*>(pass.get())->GetSetCount();

			std::array<VkDescriptorSet, 3> descriptorSets{};
			descriptorSets.fill(nullptr);

			bool bDynamic = true;
			for (int i = 0; i < setSize; ++i)
			{
				const MaterialData* matData = nullptr;
				if (i == 1)
					matData = &drawable->GetMaterialData();
				else
					matData = &mat->GetMaterialData();

				auto uniformBuffer = static_cast<VulkanUniformBuffer*>(matData->GetUniformBuffer(*pass.get(), static_cast<UniformStructLayout::Type>(i), core::ThreadType::Render));
				if (uniformBuffer == nullptr)
					descriptorSets[i] = context->GetEmptyDescriptorSet();
				else
					descriptorSets[i] = uniformBuffer->GetVkDescriptorSet();
			}
			if (descriptorSets[0] == context->GetEmptyDescriptorSet())
				bDynamic = false;

			uint32_t offset = cameraManager->GetDynamicOffset(camera);

			vkCmdBindDescriptorSets(cmd->GetCommandBuffer(),
				VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS,
				static_cast<VulkanShaderPass&>(*pass).GetPipelineLayout(), 0, setSize,
				descriptorSets.data(), bDynamic ? 1 : 0, &offset);

			if (pass->HasConstantUniform())
				vkCmdPushConstants(cmd->GetCommandBuffer(),
					static_cast<VulkanShaderPass*>(pass.get())->GetPipelineLayout(),
					VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &drawable->GetModelMatrix());

			vkCmdDrawIndexed(cmd->GetCommandBuffer(), mesh->GetIndices().size(), 1, 0, 0, 0);
		}
	}
	SH_RENDER_API void VulkanBasePass::Init(IRenderContext& context)
	{
		this->context = static_cast<VulkanContext*>(&context);
		cmd = this->context->GetCommandBuffer(core::ThreadType::Render);

		cameraManager = VulkanCameraBuffers::GetInstance();
	}
	SH_RENDER_API void VulkanBasePass::PushDrawable(Drawable* drawable)
	{
		if (!drawable->CheckAssetValid())
			return;
		if (drawable->GetMaterial()->GetShader()->GetShaderPasses(passName) == nullptr)
			return;

		drawables.push_back(drawable);
	}
	SH_RENDER_API void VulkanBasePass::ClearDrawable()
	{
		drawables.clear();
	}
	SH_RENDER_API void sh::render::vk::VulkanBasePass::RecordCommand(const Camera& camera, uint32_t imgIdx)
	{
		assert(cmd != nullptr);
		std::array<VkClearValue, 2> clear;
		clear[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
		clear[1].depthStencil = { 1.0f, 0 };

		RenderTexture* renderTexture = camera.GetRenderTexture();

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clear.size());
		renderPassInfo.pClearValues = clear.data();

		VkViewport viewport{};
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };

		VkRenderPass renderPass = VK_NULL_HANDLE;
		if (renderTexture == nullptr)
		{
			const VulkanFramebuffer* mainFramebuffer = static_cast<const VulkanFramebuffer*>(context->GetMainFramebuffer(imgIdx));
			renderPass = mainFramebuffer->GetRenderPass()->GetVkRenderPass();
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = mainFramebuffer->GetVkFramebuffer();
			renderPassInfo.renderArea.extent = context->GetSwapChain().GetSwapChainSize();

			float width = context->GetViewportEnd().x - context->GetViewportStart().x;
			float height = context->GetViewportEnd().y - context->GetViewportStart().y;
			float surfWidth = static_cast<float>(context->GetSwapChain().GetSwapChainSize().width);
			float surfHeight = static_cast<float>(context->GetSwapChain().GetSwapChainSize().height);
			viewport.x = context->GetViewportStart().x;
			viewport.y = context->GetViewportEnd().y;
			viewport.width = std::min(width, surfWidth);
			viewport.height = -std::min(height, surfHeight);

			scissor.extent = context->GetSwapChain().GetSwapChainSize();
		}
		else
		{
			auto vkFramebuffer = static_cast<VulkanFramebuffer*>(renderTexture->GetFramebuffer(core::ThreadType::Render));
			renderPass = vkFramebuffer->GetRenderPass()->GetVkRenderPass();
			renderPassInfo.renderPass = vkFramebuffer->GetRenderPass()->GetVkRenderPass();
			renderPassInfo.framebuffer = vkFramebuffer->GetVkFramebuffer();
			renderPassInfo.renderArea.extent = { vkFramebuffer->GetWidth(), vkFramebuffer->GetHeight() };

			viewport.x = 0;
			viewport.y = vkFramebuffer->GetHeight();
			viewport.width = vkFramebuffer->GetWidth();
			viewport.height = -static_cast<float>(vkFramebuffer->GetHeight());

			scissor.extent = { vkFramebuffer->GetWidth(), vkFramebuffer->GetHeight() };

			renderTexture->SetDirty();
		}
		VkCommandBuffer commandBuffer = cmd->GetCommandBuffer();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		for (auto drawable : drawables)
		{
			if (!camera.CheckRenderTag(drawable->GetRenderTagId()))
				continue;

			RenderDrawable(camera, renderPass, drawable);
		}

		vkCmdEndRenderPass(commandBuffer);
	}
	SH_RENDER_API auto sh::render::vk::VulkanBasePass::GetName() const -> const std::string&
	{
		return passName;
	}
}//namespace