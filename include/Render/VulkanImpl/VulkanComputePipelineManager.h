#pragma once
#include "Export.h"

#include "Core/NonCopyable.h"
#include "Core/Observer.hpp"

#include <map>
#include <memory>
#include <shared_mutex>
namespace sh::core
{
	class SObject;
}
namespace sh::render
{
	class ComputeShader;
}
namespace sh::render::vk
{
	class VulkanContext;
	class VulkanComputePipeline;
	class VulkanComputePipelineManager : public core::INonCopyable
	{
	public:
		SH_RENDER_API VulkanComputePipelineManager(const VulkanContext& context);
		SH_RENDER_API VulkanComputePipelineManager(VulkanComputePipelineManager&& other) = delete;

		/// @brief 파이프라인을 생성하거나 가져온다. 스레드 안전하다.
		/// @param computeShader 컴퓨트 셰이더
		SH_RENDER_API auto GetOrCreatePipeline(const ComputeShader& computeShader) -> VulkanComputePipeline&;
	private:
		const VulkanContext& ctx;

		std::shared_mutex mu;

		core::Observer<false, const core::SObject*>::Listener onShaderDestroy;
		std::map<const ComputeShader*, std::unique_ptr<VulkanComputePipeline>> pipelines;
	};
}//namespace