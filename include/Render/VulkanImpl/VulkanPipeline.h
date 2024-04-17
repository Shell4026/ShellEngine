#pragma once

#include "Export.h"
#include "VulkanConfig.h"
#include "Core/NonCopyable.h"

#include <vector>

namespace sh::render{ class VulkanShader; }

namespace sh::render::impl
{
	class VulkanSurface;

	class VulkanPipeline : public core::INonCopyable
	{
	public:
		 enum class ShaderStage {
			Vertex,
			Fragment
		};
	private:
		const VulkanSurface& surface;

		VkRenderPass renderPass;
		VkPipelineLayout pipelineLayout;
		VkPipeline pipeline;

		VkDevice device;

		const VulkanShader* shader;
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	private:
		void CreateRenderPass();
	public:
		SH_RENDER_API VulkanPipeline(const VulkanSurface& surface, const VulkanShader* shader);
		SH_RENDER_API ~VulkanPipeline();

		SH_RENDER_API auto CreateGraphicsPipeline() -> VkResult;
		SH_RENDER_API void Destroy();

		SH_RENDER_API auto GetRenderPass() const -> VkRenderPass;
		SH_RENDER_API auto GetPipeline() const -> VkPipeline;

		SH_RENDER_API void AddShaderStage(ShaderStage stage);

		SH_RENDER_API auto GetDevice() const -> VkDevice;
	};
}