#include "VulkanRenderImpl.h"
#include "VulkanCommandBuffer.h"
#include "VulkanSwapChain.h"
#include "VulkanDescriptorSet.h"
#include "VulkanVertexBuffer.h"
#include "VulkanSkinnedVertexBuffer.h"
#include "VulkanPipelineManager.h"
#include "VulkanShaderPass.h"
#include "../Material.h"
#include "../ShaderPass.h"
#include "../Drawable.h"
#include "../SkinnedMesh.h"

#include "Core/Reflection.hpp"

namespace sh::render::vk
{
	VulkanRenderImpl::VulkanRenderImpl(VulkanContext& context) :
		ctx(context)
	{
		cameraManager = VulkanCameraBuffers::GetInstance();
	}
	SH_RENDER_API void VulkanRenderImpl::EmitBarrier(CommandBuffer& _cmd, const std::vector<BarrierInfo>& barriers) const
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
				ResourceUsage u,
				VkImageLayout& layout,
				VkPipelineStageFlags& stage,
				VkAccessFlags& access)
				{
					switch (u)
					{
					case ResourceUsage::Undefined:
						layout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
						stage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
						access = 0;
						return;
					case ResourceUsage::ColorAttachment:
						layout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						stage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
						access = VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
						return;
					case ResourceUsage::SampledRead:
						layout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						stage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
						access = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;
						return;
					case ResourceUsage::Present:
						layout = VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
						stage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
						access = 0;
						return;
					case ResourceUsage::TransferSrc:
						layout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
						stage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT;
						access = VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT;
						return;
					case ResourceUsage::DepthStencilAttachment:
						layout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
						stage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
								VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
						access = VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
								 VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
						return;
					case ResourceUsage::DepthStencilSampledRead:
						layout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
						stage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
						access = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;
						return;
					}
				};

			mapUsageFn(barrier.lastUsage, srcLayout, srcStage, srcAccess);
			mapUsageFn(barrier.curUsage, dstLayout, dstStage, dstAccess);

			VulkanImageBuffer::BarrierCommand(cmd.GetCommandBuffer(), *imgBuffer, srcLayout, dstLayout, srcStage, dstStage, srcAccess, dstAccess);

			if (msaaBuffer != nullptr)
			{
				// MSAA 버퍼를 다른 용도로 쓸 일이 없을듯?
				//if (barrier.curUsage != ResourceUsage::Present)
					//VulkanImageBuffer::BarrierCommand(cmd.GetCommandBuffer(), *msaaBuffer, srcLayout, dstLayout, srcStage, dstStage, srcAccess, dstAccess);
			}
		}
	}
	namespace
	{
		struct ResolvedTarget
		{
			VulkanImageBuffer* color = nullptr;
			VulkanImageBuffer* colorMSAA = nullptr;
			VulkanImageBuffer* depth = nullptr;
			RenderTargetLayout layout{};
			uint32_t width = 0;
			uint32_t height = 0;
			bool bSwapChain = false;
			bool bDepthOnly = false;
		};

		auto ResolveTarget(VulkanContext& ctx, const RenderTarget& renderData) -> ResolvedTarget
		{
			ResolvedTarget target{};
			if (core::IsValid(renderData.target))
			{
				target.bSwapChain = false;
				target.bDepthOnly = renderData.target->IsDepthOnly();
				target.color = target.bDepthOnly ? nullptr : static_cast<VulkanImageBuffer*>(renderData.target->GetTextureBuffer());
				target.colorMSAA = static_cast<VulkanImageBuffer*>(renderData.target->GetMSAABuffer());
				target.depth = static_cast<VulkanImageBuffer*>(renderData.target->GetDepthBuffer());
				target.layout = renderData.target->GetLayout();

				VulkanImageBuffer* const sizeRef = target.color != nullptr ? target.color : target.depth;
				target.width = sizeRef->GetWidth();
				target.height = sizeRef->GetHeight();
			}
			else
			{
				target.bSwapChain = true;
				const uint32_t idx = renderData.frameIndex;
				target.colorMSAA = &ctx.GetSwapChain().GetSwapChainMSAAImages()[idx];
				target.color = &ctx.GetSwapChain().GetSwapChainImages()[idx];
				target.depth = &ctx.GetSwapChain().GetSwapChainDepthImages()[idx];
				target.layout = ctx.GetSwapChain().GetRenderTargetLayout();
				target.width = target.color->GetWidth();
				target.height = target.color->GetHeight();
			}
			return target;
		}

		auto BuildViewport(VulkanContext& ctx, const ResolvedTarget& target) -> VkViewport
		{
			VkViewport viewport{};
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			if (target.bSwapChain)
			{
				const float w = ctx.GetViewportEnd().x - ctx.GetViewportStart().x;
				const float h = ctx.GetViewportEnd().y - ctx.GetViewportStart().y;
				const float surfW = static_cast<float>(ctx.GetSwapChain().GetSwapChainSize().width);
				const float surfH = static_cast<float>(ctx.GetSwapChain().GetSwapChainSize().height);
				viewport.x = ctx.GetViewportStart().x;
				viewport.y = ctx.GetViewportEnd().y;
				viewport.width = std::min(w, surfW);
				viewport.height = -std::min(h, surfH);
			}
			else
			{
				viewport.x = 0.f;
				viewport.y = static_cast<float>(target.height);
				viewport.width = static_cast<float>(target.width);
				viewport.height = -static_cast<float>(target.height);
			}
			return viewport;
		}

		auto BuildScissor(VulkanContext& ctx, const ResolvedTarget& target) -> VkRect2D
		{
			VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent = target.bSwapChain ? ctx.GetSwapChain().GetSwapChainSize() : VkExtent2D{ target.width, target.height };
			return scissor;
		}

		auto BuildColorAttachment(const ResolvedTarget& target, const DrawList& drawList, bool bStoreImage) -> VkRenderingAttachmentInfoKHR
		{
			VkRenderingAttachmentInfoKHR info{};
			info.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			const bool bMSAA = (target.colorMSAA != nullptr);
			info.imageView = bMSAA ? target.colorMSAA->GetImageView() : target.color->GetImageView();
			info.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			info.loadOp = drawList.bClearColor ? VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR : VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;
			info.storeOp = bStoreImage ? VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE : VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
			info.clearValue.color = { { 0.f, 0.f, 0.f, 1.f } };
			if (bMSAA)
			{
				info.resolveImageView = target.color->GetImageView();
				info.resolveImageLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				info.resolveMode = VkResolveModeFlagBitsKHR::VK_RESOLVE_MODE_AVERAGE_BIT_KHR;
			}
			return info;
		}

		auto BuildDepthAttachment(const ResolvedTarget& target, const DrawList& drawList, bool bStoreImage) -> VkRenderingAttachmentInfoKHR
		{
			VkRenderingAttachmentInfoKHR info{};
			info.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
			info.imageView = target.depth->GetImageView();
			info.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			info.loadOp = drawList.bClearDepth ? VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR : VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;
			info.storeOp = bStoreImage ? VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE : VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
			info.clearValue.depthStencil = { 1.0f, 0 };
			return info;
		}

		auto HasDrawables(const DrawList& drawList) -> bool
		{
			if (std::holds_alternative<std::vector<DrawList::RenderGroup>>(drawList.renderData))
				return !std::get<std::vector<DrawList::RenderGroup>>(drawList.renderData).empty();

			return !std::get<std::vector<DrawList::RenderItem>>(drawList.renderData).empty();
		}
	}//anonymous namespace

	SH_RENDER_API void VulkanRenderImpl::RecordCommand(CommandBuffer& _cmd, const core::Name& passName, const RenderTarget& renderData, const DrawList& drawList, bool bStoreImage) const
	{
		// 드로우 목적이 아닌 경우
		if (renderData.camera == nullptr)
		{
			for (auto& call : drawList.drawCall)
				call(_cmd);
			return;
		}

		VulkanCommandBuffer& cmd = static_cast<VulkanCommandBuffer&>(_cmd);
		VkCommandBuffer commandBuffer = cmd.GetCommandBuffer();

		const ResolvedTarget target = ResolveTarget(ctx, renderData);
		const VkViewport viewport = BuildViewport(ctx, target);
		const VkRect2D scissor = BuildScissor(ctx, target);

		VkRenderingAttachmentInfoKHR colorAttach{};
		VkRenderingAttachmentInfoKHR depthAttach{};
		if (!target.bDepthOnly)
			colorAttach = BuildColorAttachment(target, drawList, bStoreImage);
		const bool bHasDepth = (target.depth != nullptr);
		if (bHasDepth)
			depthAttach = BuildDepthAttachment(target, drawList, bStoreImage);

		VkRenderingInfoKHR renderingInfo{};
		renderingInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
		renderingInfo.colorAttachmentCount = target.bDepthOnly ? 0 : 1;
		renderingInfo.pColorAttachments = target.bDepthOnly ? nullptr : &colorAttach;
		renderingInfo.pDepthAttachment = bHasDepth ? &depthAttach : nullptr;
		// depth-only RT는 view aspect=DEPTH-only이므로 stencil 첨부는 비활성화
		renderingInfo.pStencilAttachment = (bHasDepth && !target.bDepthOnly) ? &depthAttach : nullptr;
		renderingInfo.renderArea = { { 0, 0 }, { target.width, target.height } };
		renderingInfo.layerCount = 1;

		static PFN_vkCmdBeginRenderingKHR pfnBegin = (PFN_vkCmdBeginRenderingKHR)vkGetDeviceProcAddr(ctx.GetDevice(), "vkCmdBeginRenderingKHR");
		static PFN_vkCmdEndRenderingKHR pfnEnd = (PFN_vkCmdEndRenderingKHR)  vkGetDeviceProcAddr(ctx.GetDevice(), "vkCmdEndRenderingKHR");

		// 5) 기록
		pfnBegin(commandBuffer, &renderingInfo);
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		if (HasDrawables(drawList))
			RenderDrawable(cmd, passName, renderData, drawList, target.layout);

		for (auto& call : drawList.drawCall)
			call(_cmd);

		pfnEnd(commandBuffer);
	}

	void VulkanRenderImpl::BindCameraSet(VulkanCommandBuffer& cmd, VkPipelineLayout layout, const ShaderPass& pass, const Material& mat, uint32_t cameraOffset) const
	{
		// 카메라 데이터는 다이나믹 디스크립터셋
		VulkanDescriptorSet* const cameraUBO = static_cast<VulkanDescriptorSet*>(
			mat.GetMaterialData().GetShaderBinding(pass, UniformStructLayout::Usage::Camera));

		VkDescriptorSet cameraSet = cameraUBO ? cameraUBO->GetVkDescriptorSet() : ctx.GetEmptyDescriptorSet();
		const uint32_t dynamicCount = cameraUBO ? 1 : 0;

		vkCmdBindDescriptorSets(cmd.GetCommandBuffer(),
			VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, layout,
			static_cast<uint32_t>(UniformStructLayout::Usage::Camera),
			1, &cameraSet, dynamicCount, &cameraOffset);
	}
	void VulkanRenderImpl::BindMaterialSet(VulkanCommandBuffer& cmd, VkPipelineLayout layout, const ShaderPass& pass, const Material& mat) const
	{
		VulkanDescriptorSet* const materialUniformBuffer = static_cast<VulkanDescriptorSet*>(
			mat.GetMaterialData().GetShaderBinding(pass, UniformStructLayout::Usage::Material));

		VkDescriptorSet materialDescriptorSet = materialUniformBuffer ? materialUniformBuffer->GetVkDescriptorSet() : ctx.GetEmptyDescriptorSet();

		vkCmdBindDescriptorSets(cmd.GetCommandBuffer(),
			VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, layout,
			static_cast<uint32_t>(UniformStructLayout::Usage::Material),
			1, &materialDescriptorSet, 0, nullptr);
	}
	void VulkanRenderImpl::BindObjectSet(VulkanCommandBuffer& cmd, VkPipelineLayout layout, const ShaderPass& pass, Drawable& drawable) const
	{
		VulkanDescriptorSet* const objectUniformBuffer = static_cast<VulkanDescriptorSet*>(
			drawable.GetMaterialData().GetShaderBinding(pass, UniformStructLayout::Usage::Object));

		VkDescriptorSet objectDescriptorSet = objectUniformBuffer ? objectUniformBuffer->GetVkDescriptorSet() : ctx.GetEmptyDescriptorSet();

		vkCmdBindDescriptorSets(cmd.GetCommandBuffer(),
			VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, layout,
			static_cast<uint32_t>(UniformStructLayout::Usage::Object),
			1, &objectDescriptorSet, 0, nullptr);
	}
	void VulkanRenderImpl::DrawMesh(VulkanCommandBuffer& cmd, const ShaderPass& pass, const Mesh& mesh, int subMeshIndex) const
	{
		uint32_t indexCount;
		uint32_t firstIndex;
		const auto& subMeshes = mesh.GetSubMeshes();
		if (subMeshIndex >= 0 && subMeshIndex < static_cast<int>(subMeshes.size()))
		{
			indexCount = static_cast<uint32_t>(subMeshes[subMeshIndex].indexCount);
			firstIndex = static_cast<uint32_t>(subMeshes[subMeshIndex].indexOffset);
		}
		else
		{
			indexCount = static_cast<uint32_t>(mesh.GetIndices().size());
			firstIndex = 0;
		}

		const SkinnedMesh* skinnedMesh = core::reflection::Cast<const SkinnedMesh>(&mesh);
		if (skinnedMesh)
		{
			const VulkanSkinnedVertexBuffer* vkSkinnedVB =
				static_cast<VulkanSkinnedVertexBuffer*>(skinnedMesh->GetVertexBuffer());

			VkBuffer buffers[2] = {
				vkSkinnedVB->GetVertexBuffer().GetBuffer(),
				vkSkinnedVB->GetBoneBuffer().GetBuffer()
			};
			VkDeviceSize offsets[2] = { 0, 0 };
			vkCmdBindVertexBuffers(cmd.GetCommandBuffer(), 0, 2, buffers, offsets);
			vkCmdBindIndexBuffer(cmd.GetCommandBuffer(), vkSkinnedVB->GetIndexBuffer().GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
		}
		else
		{
			const VulkanVertexBuffer* vkVertexBuffer = static_cast<VulkanVertexBuffer*>(mesh.GetVertexBuffer());

			std::array<VkBuffer, 1> buffers = { vkVertexBuffer->GetVertexBuffer().GetBuffer() };

			VkDeviceSize offsets[1] = { 0 };
			vkCmdBindVertexBuffers(cmd.GetCommandBuffer(), 0, 1, buffers.data(), offsets);
			vkCmdBindIndexBuffer(cmd.GetCommandBuffer(), vkVertexBuffer->GetIndexBuffer().GetBuffer(), 0, VkIndexType::VK_INDEX_TYPE_UINT32);
		}
		vkCmdDrawIndexed(cmd.GetCommandBuffer(), indexCount, 1, firstIndex, 0, 0);
	}
	void VulkanRenderImpl::RenderDrawable(VulkanCommandBuffer& cmd, const core::Name& passName, const RenderTarget& renderData, const DrawList& drawList, const RenderTargetLayout& layout) const
	{
		std::optional<uint32_t> cameraOffsetOpt = cameraManager->GetDynamicOffset(*renderData.camera);
		if (!cameraOffsetOpt.has_value())
			return;

		VulkanPipelineManager::PipelineHandle lastPipelineHandle{ 0xffffffff, 0xffffffff };
		if (drawList.renderData.index() == 0)
		{
			for (const DrawList::RenderGroup& renderGroup : std::get<0>(drawList.renderData))
			{
				const Material* mat = renderGroup.material;
				assert(mat);

				const auto passVectorPtr = renderGroup.material->GetShader()->GetShaderPasses(passName);
				if (passVectorPtr == nullptr)
					continue;

				// 첫 번째 유효 drawable의 mesh로 스킨드 여부 판단
				bool bGroupSkinned = false;
				for (auto drawable : renderGroup.drawables)
				{
					if (drawable && drawable->GetMesh())
					{
						bGroupSkinned = (core::reflection::Cast<const SkinnedMesh>(drawable->GetMesh()) != nullptr);
						break;
					}
				}

				for (const ShaderPass& pass : *passVectorPtr)
				{
					if (pass.IsPendingKill())
						continue;

					const std::vector<uint8_t>* constantData = mat->GetConstantData(pass);

					auto pipelineHandle = ctx.GetPipelineManager().
						GetOrCreatePipelineHandle(static_cast<const VulkanShaderPass&>(pass), layout, renderGroup.topology, bGroupSkinned, constantData);

					if (lastPipelineHandle != pipelineHandle)
					{
						ctx.GetPipelineManager().BindPipeline(cmd.GetCommandBuffer(), pipelineHandle);
						lastPipelineHandle = pipelineHandle;
					}
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

						DrawMesh(cmd, pass, *mesh, drawable->GetSubMeshIndex());
					}
				}
			}
		}
		else
		{
			for (const DrawList::RenderItem& renderItem : std::get<1>(drawList.renderData))
			{
				const Material* mat = renderItem.material;
				assert(mat);

				const auto passVectorPtr = mat->GetShader()->GetShaderPasses(passName);
				if (passVectorPtr == nullptr)
					continue;

				bool bItemSkinned = false;
				if (renderItem.drawable && renderItem.drawable->GetMesh())
					bItemSkinned = (core::reflection::Cast<const SkinnedMesh>(renderItem.drawable->GetMesh()) != nullptr);

				for (const ShaderPass& pass : *passVectorPtr)
				{
					if (pass.IsPendingKill())
						continue;

					const std::vector<uint8_t>* constantData = mat->GetConstantData(pass);

					auto pipelineHandle = ctx.GetPipelineManager().
						GetOrCreatePipelineHandle(static_cast<const VulkanShaderPass&>(pass), layout, renderItem.topology, bItemSkinned, constantData);

					if (lastPipelineHandle != pipelineHandle)
					{
						ctx.GetPipelineManager().BindPipeline(cmd.GetCommandBuffer(), pipelineHandle);
						lastPipelineHandle = pipelineHandle;
					}
					VkPipelineLayout pipelineLayout = static_cast<const VulkanShaderPass&>(pass).GetPipelineLayout();
					uint32_t setSize = static_cast<const VulkanShaderPass&>(pass).GetSetCount();

					// set = 0 카메라
					// set = 1 객체 고유
					// set = 2 메테리얼
					if (setSize > 0)
						BindCameraSet(cmd, pipelineLayout, pass, *mat, cameraOffsetOpt.value());
					if (setSize > 2)
						BindMaterialSet(cmd, pipelineLayout, pass, *mat);

					if (renderItem.drawable == nullptr)
						continue;
					const Mesh* mesh = renderItem.drawable->GetMesh();
					if (!core::IsValid(mesh))
						continue;

					if (setSize > 1)
						BindObjectSet(cmd, pipelineLayout, pass, *renderItem.drawable);

					if (pass.HasConstantUniform())
						vkCmdPushConstants(cmd.GetCommandBuffer(), pipelineLayout,
							VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT,
							0, sizeof(glm::mat4),
							&renderItem.drawable->GetModelMatrix(core::ThreadType::Render));

					DrawMesh(cmd, pass, *mesh, renderItem.drawable->GetSubMeshIndex());
				}
			}
		}
	}
}//namespace