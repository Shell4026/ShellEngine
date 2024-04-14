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
	public:
		~VulkanPipeline();

		void CreateRenderPass(const VulkanSurface* surface);
		void CreateGraphicsPipeline(const VulkanShader* shader, const VulkanSurface* surface);
		void Destroy();
	};
}