#pragma once

#include "Export.h"
#include "VulkanConfig.h"

#include <vector>

namespace sh::render{ class VulkanShader; }

namespace sh::render::impl
{
	class VulkanSurface;

	class SH_RENDER_API VulkanPipeline
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
		VulkanPipeline(const VulkanShader* shader);
		~VulkanPipeline();

		auto CreateGraphicsPipeline(const VulkanSurface* surface) -> VkResult;
		void Destroy();

		auto GetRenderPass() const -> VkRenderPass;
		auto GetPipeline() const -> VkPipeline;

		void AddShaderStage(ShaderStage stage);
	};
}