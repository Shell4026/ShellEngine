#pragma once
#include "../Export.h"
#include "../IRenderImpl.h"
#include "../RenderTarget.h"
#include "VulkanContext.h"
#include "VulkanCameraBuffers.h"

namespace sh::render
{
	class ShaderPass;
}

namespace sh::render::vk
{
	class VulkanCommandBuffer;
	class VulkanCameraBuffers;

	class VulkanRenderImpl : public IRenderImpl
	{
	public:
		SH_RENDER_API VulkanRenderImpl(VulkanContext& context);
		SH_RENDER_API void EmitBarrier(CommandBuffer& cmd, const std::vector<BarrierInfo>& barriers) override;
		SH_RENDER_API void RecordCommand(CommandBuffer& cmd, const core::Name& passName, const RenderTarget& renderData, const DrawList& drawList) override;
	private:
		void BindCameraSet(VulkanCommandBuffer& cmd, VkPipelineLayout layout, const ShaderPass& pass, const Material& mat, uint32_t cameraOffset);
		void BindMaterialSet(VulkanCommandBuffer& cmd, VkPipelineLayout layout, const ShaderPass& pass, const Material& mat);
		void BindObjectSet(VulkanCommandBuffer& cmd, VkPipelineLayout layout, const ShaderPass& pass, Drawable& drawable);
		void DrawMesh(VulkanCommandBuffer& cmd, const ShaderPass& pass, const Mesh& mesh);
		void RenderDrawable(VulkanCommandBuffer& cmd, const core::Name& passName, const RenderTarget& renderData, const DrawList& drawList, const RenderTargetLayout& layout);
	private:
		VulkanContext& ctx;
		VulkanCameraBuffers* cameraManager = nullptr;
	};
}//namespace