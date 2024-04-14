#pragma once

#include "Export.h"
#include "VulkanConfig.h"

namespace sh::render{ class VulkanShader; }

namespace sh::render::impl
{
	class VulkanSurface;

	class SH_RENDER_API VulkanPipeline
	{
	private:
		VkRenderPass renderPass;
		VkPipelineLayout pipelineLayout;
		VkPipeline pipeline;

		VkDevice device;
	private:
		void CreateRenderPass(const VulkanSurface* surface);
	public:
		~VulkanPipeline();

		auto CreateGraphicsPipeline(const VulkanShader* shader, const VulkanSurface* surface) -> VkResult;
		void Destroy();

		auto GetRenderPass() const -> VkRenderPass;
		auto GetPipeline() const -> VkPipeline;
	};
}