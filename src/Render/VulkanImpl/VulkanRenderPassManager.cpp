#include "VulkanRenderPassManager.h"

namespace sh::render::vk
{
	auto VulkanRenderPassManager::CreateRenderPass(const VulkanRenderPass::Config& config) -> VulkanRenderPass
	{
		VulkanRenderPass renderPass{ context };
		renderPass.Create(config);
		return renderPass;
	}

	VulkanRenderPassManager::VulkanRenderPassManager(const VulkanContext& context) :
		context(context)
	{
	}

	SH_RENDER_API auto VulkanRenderPassManager::GetOrCreateRenderPass(const VulkanRenderPass::Config& config) -> VulkanRenderPass&
	{
		std::lock_guard<std::mutex> lock{ mu };
		auto it = renderPasses.find(config);
		if (it == renderPasses.end())
			return renderPasses.insert_or_assign(config, CreateRenderPass(config)).first->second;
		else
			return it->second;
	}
}//namespace