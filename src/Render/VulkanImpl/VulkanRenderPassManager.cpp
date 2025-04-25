#include "VulkanRenderPassManager.h"

#include <mutex> // std::unique_lock
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
		{
			std::shared_lock<std::shared_mutex> readLock{ mu };
			auto it = renderPasses.find(config);
			if (it != renderPasses.end())
				return it->second;
		}
		{
			std::unique_lock<std::shared_mutex> writeLock{ mu };
			auto it = renderPasses.find(config);
			if (it != renderPasses.end())
				return it->second;
			
			return renderPasses.insert_or_assign(config, CreateRenderPass(config)).first->second;
		}
	}
}//namespace