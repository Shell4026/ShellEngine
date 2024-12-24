#pragma once

#include "Render/Export.h"
#include "VulkanConfig.h"
#include "Core/NonCopyable.h"

#include <vector>
#include <initializer_list>

namespace sh::render{ class VulkanShader; }

namespace sh::render::vk
{
	class VulkanSurface;

	class VulkanPipeline : public core::INonCopyable
	{
	public:
		 enum class ShaderStage {
			Vertex,
			Fragment
		};

		 enum class CullMode
		 {
			 Off,
			 Front,
			 Back
		 };

		 enum class Topology
		 {
			 Point,
			 Line,
			 Triangle
		 };
	private:
		VkDevice device;
		VkRenderPass renderPass;
		const VulkanShader* shader;

		VkPipeline pipeline;

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

		CullMode cullMode;
		Topology topology;

		float lineWidth = 1.f;
	public:
		SH_RENDER_API VulkanPipeline(VkDevice device, VkRenderPass renderPass);
		SH_RENDER_API VulkanPipeline(VulkanPipeline&& other) noexcept;
		SH_RENDER_API ~VulkanPipeline();

		SH_RENDER_API auto Build(VkPipelineLayout layout) -> VkResult;
		SH_RENDER_API void Clean();

		SH_RENDER_API auto GetPipeline() const -> VkPipeline;

		SH_RENDER_API auto AddShaderStage(ShaderStage stage) -> VulkanPipeline&;
		SH_RENDER_API auto ResetShaderStage() -> VulkanPipeline&;

		SH_RENDER_API auto AddBindingDescription(const VkVertexInputBindingDescription& bindingDescription) -> VulkanPipeline&;
		SH_RENDER_API auto ResetBindingDescription() -> VulkanPipeline&;

		SH_RENDER_API auto AddAttributeDescription(const VkVertexInputAttributeDescription& attributeDescription) -> VulkanPipeline&;
		SH_RENDER_API auto ResetAttributeDescription() -> VulkanPipeline&;

		SH_RENDER_API auto SetShader(const VulkanShader* shader) -> VulkanPipeline&;

		SH_RENDER_API auto SetCullMode(CullMode mode) -> VulkanPipeline&;

		SH_RENDER_API auto SetTopology(Topology topology) -> VulkanPipeline&;
		SH_RENDER_API auto GetTopology() const -> Topology;

		SH_RENDER_API void SetLineWidth(float width);
		SH_RENDER_API auto GetLineWidth() const -> float;
	};
}