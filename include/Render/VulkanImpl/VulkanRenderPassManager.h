#pragma once
#include "Export.h"
#include "VulkanConfig.h"
#include "VulkanRenderPass.h"

#include "Core/SContainer.hpp"
namespace sh::render::vk
{
	class VulkanContext;
	class VulkanRenderPassManager
	{
	private:
		const VulkanContext& context;

		core::SHashMap<VulkanRenderPass::Config, VulkanRenderPass, 16, VulkanRenderPass::ConfigHasher> renderPasses;
	private:
		auto CreateRenderPass(const VulkanRenderPass::Config& config) -> VulkanRenderPass;
	public:
		SH_RENDER_API VulkanRenderPassManager(const VulkanContext& context);
		SH_RENDER_API auto GetOrCreateRenderPass(const VulkanRenderPass::Config& config) -> VulkanRenderPass&;
	};
}//namespace