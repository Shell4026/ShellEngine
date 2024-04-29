#pragma once

#include "Export.h"
#include "VulkanConfig.h"
#include "Core/NonCopyable.h"

#include <vector>
#include <initializer_list>

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
		VkDevice device;
		VkRenderPass renderPass;
		const VulkanShader* shader;

		VkPipelineLayout pipelineLayout;
		VkPipeline pipeline;

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

		int viewportX, viewportY;
	public:
		SH_RENDER_API VulkanPipeline(VkDevice device, VkRenderPass renderPass, const VulkanShader* shader);
		SH_RENDER_API ~VulkanPipeline();

		SH_RENDER_API auto Build() -> VkResult;
		SH_RENDER_API void Clean();

		SH_RENDER_API auto GetPipeline() const -> VkPipeline;

		SH_RENDER_API auto AddShaderStage(ShaderStage stage) -> VulkanPipeline&;
		SH_RENDER_API auto ResetShaderStage() -> VulkanPipeline&;

		SH_RENDER_API auto AddBindingDescription(const VkVertexInputBindingDescription& bindingDescription) -> VulkanPipeline&;
		SH_RENDER_API auto ResetBindingDescription() -> VulkanPipeline&;

		SH_RENDER_API auto AddAttributeDescription(const VkVertexInputAttributeDescription& attributeDescription) -> VulkanPipeline&;
		SH_RENDER_API auto ResetAttributeDescription() -> VulkanPipeline&;
	};
}