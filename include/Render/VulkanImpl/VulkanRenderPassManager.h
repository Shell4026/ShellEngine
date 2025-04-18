#pragma once
#include "Export.h"
#include "VulkanConfig.h"
#include "VulkanRenderPass.h"

#include "Core/SContainer.hpp"

#include <shared_mutex>
namespace sh::render::vk
{
	class VulkanContext;
	class VulkanRenderPassManager
	{
	private:
		const VulkanContext& context;

		core::SHashMap<VulkanRenderPass::Config, VulkanRenderPass, 16, VulkanRenderPass::ConfigHasher> renderPasses;

		std::shared_mutex mu;
	private:
		auto CreateRenderPass(const VulkanRenderPass::Config& config) -> VulkanRenderPass;
	public:
		SH_RENDER_API VulkanRenderPassManager(const VulkanContext& context);

		/// @brief 렌더 패스를 가져오거나 새로 만든다. 스레드 안전하다.
		/// @param config 설정값
		SH_RENDER_API auto GetOrCreateRenderPass(const VulkanRenderPass::Config& config) -> VulkanRenderPass&;
	};
}//namespace