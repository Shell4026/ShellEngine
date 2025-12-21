#include "VulkanRenderImpl.h"
#include "VulkanCommandBuffer.h"
#include "VulkanSwapChain.h"
#include "VulkanUniformBuffer.h"
#include "VulkanVertexBuffer.h"
#include "VulkanPipelineManager.h"
#include "VulkanShaderPass.h"
#include "../Material.h"
#include "../ShaderPass.h"
#include "../Drawable.h"

namespace sh::render::vk
{
	VulkanRenderImpl::VulkanRenderImpl(VulkanContext& context) :
		ctx(context)
	{
		cameraManager = VulkanCameraBuffers::GetInstance();
	}
	SH_RENDER_API void VulkanRenderImpl::EmitBarrier(CommandBuffer& _cmd, const std::vector<BarrierInfo>& barriers)
	{
		auto& cmd = static_cast<VulkanCommandBuffer&>(_cmd);
		for (const BarrierInfo& barrier : barriers)
		{
			VulkanImageBuffer* imgBuffer = nullptr;
			VulkanImageBuffer* msaaBuffer = nullptr;

			if (barrier.target.index() == 0)
			{
				imgBuffer = (static_cast<VulkanImageBuffer*>(std::get<0>(barrier.target)->GetTextureBuffer()));
				msaaBuffer = (static_cast<VulkanImageBuffer*>(std::get<0>(barrier.target)->GetMSAABuffer()));
			}
			else
			{
				const uint32_t imgIdx = std::get<1>(barrier.target);
				imgBuffer = &ctx.GetSwapChain().GetSwapChainImages()[imgIdx];
				msaaBuffer = &ctx.GetSwapChain().GetSwapChainMSAAImages()[imgIdx];
			}

			VkImageLayout srcLayout, dstLayout;
			VkPipelineStageFlags srcStage, dstStage;
			VkAccessFlags srcAccess, dstAccess;

			auto mapUsageFn = 
				[&](
				ImageUsage u,
				VkImageLayout& layout,
				VkPipelineStageFlags& stage,
				VkAccessFlags& access)
				{
					switch (u)
					{
					case ImageUsage::Undefined:
						layout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
						stage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
						access = 0;
						return;
					case ImageUsage::ColorAttachment:
						layout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						stage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
						access = VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
						return;
					case ImageUsage::SampledRead:
						layout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						stage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
						access = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;
						return;
					case ImageUsage::Present:
						layout = VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
						stage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
						access = 0;
						return;
					case ImageUsage::Src:
						layout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
						stage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT;
						access = VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT;
					}
				};

			mapUsageFn(barrier.lastUsage, srcLayout, srcStage, srcAccess);
			mapUsageFn(barrier.curUsage, dstLayout, dstStage, dstAccess);

			VulkanImageBuffer::BarrierCommand(cmd.GetCommandBuffer(), *imgBuffer, srcLayout, dstLayout, srcStage, dstStage, srcAccess, dstAccess);

			if (msaaBuffer != nullptr)
			{
				// MSAA 버퍼를 다른 용도로 쓸 일이 없을듯?
				//if (barrier.curUsage != ImageUsage::Present)
					//VulkanImageBuffer::BarrierCommand(cmd.GetCommandBuffer(), *msaaBuffer, srcLayout, dstLayout, srcStage, dstStage, srcAccess, dstAccess);
			}
		}
	}
	SH_RENDER_API void VulkanRenderImpl::RecordCommand(CommandBuffer& _cmd, const core::Name& passName, const RenderTarget& renderData, const DrawList& drawList)
	{
		auto& cmd = static_cast<VulkanCommandBuffer&>(_cmd);

		const float width = ctx.GetViewportEnd().x - ctx.GetViewportStart().x;
		const float height = ctx.GetViewportEnd().y - ctx.GetViewportStart().y;
		const float surfWidth = static_cast<float>(ctx.GetSwapChain().GetSwapChainSize().width);
		const float surfHeight = static_cast<float>(ctx.GetSwapChain().GetSwapChainSize().height);

		// 드로우 목적이 아닌 경우
		if (renderData.camera == nullptr)
		{
			for (auto& call : drawList.drawCall)
				call(_cmd);
			return;
		}

		VkViewport viewport{};
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		viewport.x = ctx.GetViewportStart().x;
		viewport.y = ctx.GetViewportEnd().y;
		viewport.width = std::min(width, surfWidth);
		viewport.height = -std::min(height, surfHeight);

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = ctx.GetSwapChain().GetSwapChainSize();

		VulkanImageBuffer* imgBufferMSAA = nullptr;
		VulkanImageBuffer* imgBuffer = nullptr;
		VulkanImageBuffer* depthBuffer = nullptr;

		RenderTargetLayout rtLayout;

		if (core::IsValid(renderData.target))
		{
			float w = renderData.target->GetSize().x;
			float h = renderData.target->GetSize().y;

			viewport.x = 0;
			viewport.y = h;
			viewport.width = w;
			viewport.height = -h;
			scissor.extent = { static_cast<uint32_t>(w), static_cast<uint32_t>(h) };

			imgBuffer = static_cast<VulkanImageBuffer*>(renderData.target->GetTextureBuffer());
			imgBufferMSAA = static_cast<VulkanImageBuffer*>(renderData.target->GetMSAABuffer());
			depthBuffer = static_cast<VulkanImageBuffer*>(renderData.target->GetDepthBuffer());

			rtLayout = renderData.target->GetLayout();
		}
		else
		{
			imgBufferMSAA = &ctx.GetSwapChain().GetSwapChainMSAAImages()[renderData.frameIndex];
			imgBuffer = &ctx.GetSwapChain().GetSwapChainImages()[renderData.frameIndex];
			depthBuffer = &ctx.GetSwapChain().GetSwapChainDepthImages()[renderData.frameIndex];

			rtLayout = ctx.GetSwapChain().GetRenderTargetLayout();
		}

		VkCommandBuffer commandBuffer = cmd.GetCommandBuffer();

		const bool bMSAA = imgBufferMSAA != nullptr;

		VkRenderingAttachmentInfoKHR colorAttachment{};
		colorAttachment.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		colorAttachment.imageView = bMSAA ? imgBufferMSAA->GetImageView() : imgBuffer->GetImageView();
		colorAttachment.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = drawList.bClearColor ? VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR : VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE; // 다음 패스에서 다시 읽을 수도 있음
		colorAttachment.clearValue = { 0.f, 0.f, 0.f, 1.f };
		if (bMSAA)
		{
			colorAttachment.resolveImageView = imgBuffer->GetImageView();
			colorAttachment.resolveImageLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorAttachment.resolveMode = VkResolveModeFlagBitsKHR::VK_RESOLVE_MODE_AVERAGE_BIT_KHR;
		}

		VkRenderingAttachmentInfoKHR depthAttachment{};
		if (depthBuffer != nullptr)
		{
			depthAttachment.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			depthAttachment.imageView = depthBuffer->GetImageView();
			depthAttachment.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.clearValue = { 1.0f, 0.f };
		}

		VkRenderingInfoKHR renderingInfo{};
		renderingInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachments = &colorAttachment;
		renderingInfo.pDepthAttachment = depthBuffer != nullptr ? &depthAttachment : nullptr;
		renderingInfo.pStencilAttachment = depthBuffer != nullptr ? &depthAttachment : nullptr;
		renderingInfo.renderArea = { 0, 0, imgBuffer->GetWidth(), imgBuffer->GetHeight() };
		renderingInfo.layerCount = 1;

		static PFN_vkCmdBeginRenderingKHR  pfnCmdBeginRenderingKHR = (PFN_vkCmdBeginRenderingKHR)vkGetDeviceProcAddr(ctx.GetDevice(), "vkCmdBeginRenderingKHR");
		pfnCmdBeginRenderingKHR(commandBuffer, &renderingInfo);
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		if (!drawList.groups.empty())
			RenderDrawable(cmd, passName, renderData, drawList, rtLayout);

		for (auto& call : drawList.drawCall)
			call(_cmd);

		static PFN_vkCmdEndRenderingKHR pfnCmdEndRenderingKHR = (PFN_vkCmdEndRenderingKHR)vkGetDeviceProcAddr(ctx.GetDevice(), "vkCmdEndRenderingKHR");;
		pfnCmdEndRenderingKHR(commandBuffer);
	}
	void VulkanRenderImpl::BindCameraSet(VulkanCommandBuffer& cmd, VkPipelineLayout layout, const ShaderPass& pass, const Material& mat, uint32_t cameraOffset)
	{
		// 카메라 데이터는 다이나믹 디스크립터셋
		auto cameraUBO = static_cast<VulkanUniformBuffer*>(
			mat.GetMaterialData().GetUniformBuffer(pass, UniformStructLayout::Type::Camera));

		VkDescriptorSet cameraSet = cameraUBO ? cameraUBO->GetVkDescriptorSet() : ctx.GetEmptyDescriptorSet();
		uint32_t dynamicCount = cameraUBO ? 1 : 0;

		vkCmdBindDescriptorSets(cmd.GetCommandBuffer(),
			VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, layout,
			static_cast<uint32_t>(UniformStructLayout::Type::Camera),
			1, &cameraSet, dynamicCount, &cameraOffset);
	}
	void VulkanRenderImpl::BindMaterialSet(VulkanCommandBuffer& cmd, VkPipelineLayout layout, const ShaderPass& pass, const Material& mat)
	{
		auto materialUniformBuffer = static_cast<VulkanUniformBuffer*>(
			mat.GetMaterialData().GetUniformBuffer(pass, UniformStructLayout::Type::Material));

		VkDescriptorSet materialDescriptorSet = materialUniformBuffer ? materialUniformBuffer->GetVkDescriptorSet() : ctx.GetEmptyDescriptorSet();

		vkCmdBindDescriptorSets(cmd.GetCommandBuffer(),
			VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, layout,
			static_cast<uint32_t>(UniformStructLayout::Type::Material),
			1, &materialDescriptorSet, 0, nullptr);
	}
	void VulkanRenderImpl::BindObjectSet(VulkanCommandBuffer& cmd, VkPipelineLayout layout, const ShaderPass& pass, Drawable& drawable)
	{
		auto objectUniformBuffer = static_cast<VulkanUniformBuffer*>(
			drawable.GetMaterialData().GetUniformBuffer(pass, UniformStructLayout::Type::Object));

		VkDescriptorSet objectDescriptorSet = objectUniformBuffer ? objectUniformBuffer->GetVkDescriptorSet() : ctx.GetEmptyDescriptorSet();

		vkCmdBindDescriptorSets(cmd.GetCommandBuffer(),
			VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, layout,
			static_cast<uint32_t>(UniformStructLayout::Type::Object),
			1, &objectDescriptorSet, 0, nullptr);
	}
	void VulkanRenderImpl::DrawMesh(VulkanCommandBuffer& cmd, const ShaderPass& pass, const Mesh& mesh)
	{
		const VulkanVertexBuffer* vkVertexBuffer = static_cast<VulkanVertexBuffer*>(mesh.GetVertexBuffer());

		std::array<VkBuffer, 1> buffers = { vkVertexBuffer->GetVertexBuffer().GetBuffer() };

		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(cmd.GetCommandBuffer(), 0, 1, buffers.data(), offsets);
		vkCmdBindIndexBuffer(cmd.GetCommandBuffer(), vkVertexBuffer->GetIndexBuffer().GetBuffer(), 0, VkIndexType::VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cmd.GetCommandBuffer(), static_cast<uint32_t>(mesh.GetIndices().size()), 1, 0, 0, 0);
	}
	void VulkanRenderImpl::RenderDrawable(VulkanCommandBuffer& cmd, const core::Name& passName, const RenderTarget& renderData, const DrawList& drawList, const RenderTargetLayout& layout)
	{
		auto cameraOffsetOpt = cameraManager->GetDynamicOffset(*renderData.camera);
		if (!cameraOffsetOpt.has_value())
			return;

		// 렌더 그룹은 메테리얼별로 나눠져있음
		for (const DrawList::Group& renderGroup : drawList.groups)
		{
			const Material* mat = renderGroup.material;
			assert(mat);

			const auto passVectorPtr = renderGroup.material->GetShader()->GetShaderPasses(passName);
			if (passVectorPtr == nullptr)
				continue;
			for (const ShaderPass& pass : *passVectorPtr)
			{
				if (pass.IsPendingKill())
					continue;

				const std::vector<uint8_t>* constantData = mat->GetConstantData(pass);

				auto pipelineHandle = ctx.GetPipelineManager().
					GetOrCreatePipelineHandle(static_cast<const VulkanShaderPass&>(pass), layout, renderGroup.topology, constantData);

				ctx.GetPipelineManager().BindPipeline(cmd.GetCommandBuffer(), pipelineHandle);
				VkPipelineLayout pipelineLayout = static_cast<const VulkanShaderPass&>(pass).GetPipelineLayout();
				uint32_t setSize = static_cast<const VulkanShaderPass&>(pass).GetSetCount();

				// set = 0 카메라
				// set = 1 객체 고유
				// set = 2 메테리얼
				if (setSize > 0)
					BindCameraSet(cmd, pipelineLayout, pass, *mat, cameraOffsetOpt.value());
				if (setSize > 2)
					BindMaterialSet(cmd, pipelineLayout, pass, *mat);

				for (auto drawable : renderGroup.drawables)
				{
					if (drawable == nullptr)
						continue;
					const Mesh* mesh = drawable->GetMesh();
					if (!core::IsValid(mesh))
						continue;

					if (setSize > 1)
						BindObjectSet(cmd, pipelineLayout, pass, *drawable);

					if (pass.HasConstantUniform())
						vkCmdPushConstants(cmd.GetCommandBuffer(), pipelineLayout,
							VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT,
							0, sizeof(glm::mat4),
							&drawable->GetModelMatrix(core::ThreadType::Render));

					DrawMesh(cmd, pass, *mesh);
				}
			}
		}
	}
}//namespace