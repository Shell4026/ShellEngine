#pragma once
#include "../Export.h"
#include "VulkanPipeline.h"
#include "VulkanConfig.h"
#include "../Mesh.h"
#include "../StencilState.h"
#include "../RenderTarget.h"

#include "Core/SContainer.hpp"
#include "Core/Util.h"
#include "Core/NonCopyable.h"
#include "Core/Observer.hpp"

#include <memory>
#include <stack>
#include <shared_mutex>
namespace sh::render
{
	class Shader;
}
namespace sh::render::vk
{
	class VulkanContext;

	/// @brief Vulkan 파이프라인을 관리하는 클래스.
	class VulkanPipelineManager : public core::INonCopyable
	{
	public:
		struct PipelineHandle
		{
			uint32_t index = 0;
			uint32_t generation = 0;
			auto operator==(const PipelineHandle& other) const -> bool
			{
				return index == other.index && generation == other.generation;
			}
			auto operator!=(const PipelineHandle& other) const -> bool
			{
				return !operator==(other);
			}
		};
	public:
		SH_RENDER_API VulkanPipelineManager(const VulkanContext& context);
		SH_RENDER_API VulkanPipelineManager(VulkanPipelineManager&& other) = delete;
		/// @brief 파이프라인을 생성하거나 가져온다. 스레드 안전하다.
		/// @param pass 렌더 패스
		/// @param shader 셰이더
		/// @param topology 메쉬 토폴로지
		/// @param constPtr 상수 데이터 포인터
		/// @return 파이프라인 핸들
		SH_RENDER_API auto GetOrCreatePipelineHandle(
			const VulkanShaderPass& shader,
			const RenderTargetLayout& renderTargetLayout, 
			Mesh::Topology topology, 
			const std::vector<uint8_t>* constDataPtr = nullptr) -> PipelineHandle;

		/// @brief vkCmdBindPipeline()의 래퍼 함수. 스레드 안전하다.
		/// @param cmd 커맨드 버퍼
		/// @param handle 파이프라인 핸들
		SH_RENDER_API bool BindPipeline(VkCommandBuffer cmd, PipelineHandle handle);
	private:
		auto BuildPipeline(
			const VulkanShaderPass& shader,
			const RenderTargetLayout& renderTargetLayout,
			Mesh::Topology topology,
			const std::vector<uint8_t>* constDataPtr) -> std::unique_ptr<VulkanPipeline>;

		auto ConvertStencilState(const StencilState& stencilState) const->VkStencilOpState;
	private:
		struct PipelineInfo
		{
			const VulkanShaderPass* shader;
			RenderTargetLayout renderTargetLayout;
			Mesh::Topology topology;
			std::size_t constantHash = 0;

			bool operator==(const PipelineInfo& other) const
			{
				return renderTargetLayout == other.renderTargetLayout && shader == other.shader && topology == other.topology && constantHash == other.constantHash;
			}
		};
		struct PipelineInfoHasher
		{
			auto operator()(const PipelineInfo& info) const -> std::size_t
			{
				using namespace sh::core;
				std::hash<const void*> hasher;
				std::hash<int> intHasher;
				std::hash<std::size_t> sizeHasher;
				std::hash<RenderTargetLayout> layoutHasher;

				std::size_t hash = layoutHasher(info.renderTargetLayout);
				hash = Util::CombineHash(hash, hasher(info.shader));
				hash = Util::CombineHash(hash, intHasher(static_cast<int>(info.topology)));
				hash = Util::CombineHash(hash, sizeHasher(info.constantHash));

				return hash;
			}
		};
		struct Pipeline
		{
			std::unique_ptr<VulkanPipeline> pipelinePtr;
			uint32_t generation;
		};

		const VulkanContext& context;

		std::vector<Pipeline> pipelines;
		std::vector<PipelineInfo> pipelinesInfo;
		std::unordered_map<PipelineInfo, uint32_t, PipelineInfoHasher> infoIdx;
		std::unordered_map<const VulkanShaderPass*, std::vector<std::size_t>> shaderIdxs;
		std::queue<uint32_t> emptyIdx;

		std::shared_mutex mu;

		core::Observer<false, const core::SObject*>::Listener shaderDestroyedListener;
	};
}//namespace