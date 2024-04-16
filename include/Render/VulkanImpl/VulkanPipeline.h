#pragma once

#include "Export.h"
#include "VulkanConfig.h"

#include <vector>

namespace sh::render{ class VulkanShader; }

namespace sh::render::impl
{
	class VulkanSurface;

	class VulkanPipeline
	{
	public:
		 enum class ShaderStage {
			Vertex,
			Fragment
		};
	private:
		VkRenderPass renderPass;
		VkPipelineLayout pipelineLayout;
		VkPipeline pipeline;

		VkDevice device;

		const VulkanShader* shader;
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	private:
		void CreateRenderPass(const VulkanSurface* surface);
	public:
		SH_RENDER_API VulkanPipeline(const VulkanShader* shader);
		SH_RENDER_API ~VulkanPipeline();

		SH_RENDER_API auto CreateGraphicsPipeline(const VulkanSurface* surface) -> VkResult;
		SH_RENDER_API void Destroy();

		SH_RENDER_API auto GetRenderPass() const -> VkRenderPass;
		SH_RENDER_API auto GetPipeline() const -> VkPipeline;

		SH_RENDER_API void AddShaderStage(ShaderStage stage);
	};
}