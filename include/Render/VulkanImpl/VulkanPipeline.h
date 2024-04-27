﻿#pragma once

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
		const VulkanSurface& surface;
		VkRenderPass renderPass;

		VkPipelineLayout pipelineLayout;
		VkPipeline pipeline;

		VkDevice device;

		const VulkanShader* shader;

		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		VkVertexInputBindingDescription bindingDescription;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	private:
		void CreateRenderPass();
	public:
		SH_RENDER_API VulkanPipeline(const VulkanSurface& surface, const VulkanShader* shader, VkRenderPass renderPass);
		SH_RENDER_API ~VulkanPipeline();

		SH_RENDER_API auto Build() -> VkResult;
		SH_RENDER_API void Clean();

		SH_RENDER_API auto GetPipeline() const -> VkPipeline;

		SH_RENDER_API auto AddShaderStage(ShaderStage stage) -> VulkanPipeline&;
		SH_RENDER_API auto ResetShaderStage() -> VulkanPipeline&;
		SH_RENDER_API auto SetBindingDescription(const VkVertexInputBindingDescription& bindingDescription) -> VulkanPipeline&;
		SH_RENDER_API auto SetAttributeDescription(const std::initializer_list<VkVertexInputAttributeDescription>& attributeDescription) -> VulkanPipeline&;

		SH_RENDER_API auto GetDevice() const -> VkDevice;
	};
}