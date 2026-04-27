#include "Render/VulkanImpl/VulkanComputePipelineManager.h"
#include "Render/VulkanImpl/VulkanComputePipeline.h"
#include "Render/VulkanImpl/VulkanContext.h"
#include "Render/ComputeShader.h"
namespace sh::render::vk
{
	VulkanComputePipelineManager::VulkanComputePipelineManager(const VulkanContext& context) :
		ctx(context)
	{
		onShaderDestroy.SetCallback(
			[this](const core::SObject* shader)
			{
				std::unique_lock<std::shared_mutex> writeLock{ mu };
				auto it = pipelines.find(static_cast<const ComputeShader*>(shader));
				if (it != pipelines.end())
					pipelines.erase(it);
			}
		);
	}
	SH_RENDER_API auto VulkanComputePipelineManager::GetOrCreatePipeline(const ComputeShader& computeShader) -> VulkanComputePipeline&
	{
		{
			std::shared_lock<std::shared_mutex> readLock{ mu };
			auto it = pipelines.find(&computeShader);
			if (it != pipelines.end())
				return *it->second;
		}
		{
			std::unique_lock<std::shared_mutex> writeLock{ mu };
			auto it = pipelines.find(&computeShader);
			if (it != pipelines.end())
				return *it->second;
			computeShader.onDestroy.Register(onShaderDestroy);
			auto result = pipelines.insert({ &computeShader, std::make_unique<VulkanComputePipeline>(ctx, computeShader) });
			VulkanComputePipeline& pipeline = *result.first->second;
			pipeline.Build();
			return pipeline;
		}
	}
}//namespace