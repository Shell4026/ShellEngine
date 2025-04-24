#pragma once
#include "Export.h"
#include "VulkanPipeline.h"
#include "VulkanConfig.h"
#include "Render/Mesh.h"
#include "Render/StencilState.h"

#include "Core/SContainer.hpp"
#include "Core/Util.h"
#include "Core/NonCopyable.h"

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
	class VulkanRenderPass;

	/// @brief Vulkan 파이프라인을 관리하는 클래스.
	class VulkanPipelineManager : public core::INonCopyable
	{
	private:
		struct PipelineInfo
		{
			const VkRenderPass pass;
			const VulkanShaderPass* shader;
			Mesh::Topology topology;

			bool operator==(const PipelineInfo& other) const
			{
				return pass == other.pass && shader == other.shader && topology == other.topology;
			}
		};
		struct PipelineInfoHasher
		{
			auto operator()(const PipelineInfo& info) const -> std::size_t
			{
				using namespace sh::core;
				std::hash<const void*> hasher;
				std::hash<int> intHasher;

				std::size_t hash = hasher(info.pass);
				hash = Util::CombineHash(hash, hasher(info.shader));
				hash = Util::CombineHash(hash, intHasher(static_cast<int>(info.topology)));
				return hash;
			}
		};
		const VulkanContext& context;
		VkDevice device = nullptr;
		
		std::vector<std::unique_ptr<VulkanPipeline>> pipelines;
		std::vector<PipelineInfo> pipelinesInfo;
		std::unordered_map<PipelineInfo, std::size_t, PipelineInfoHasher> infoIdx;
		std::unordered_map<VkRenderPass, std::vector<std::size_t>> renderpassIdxs;
		std::unordered_map<const VulkanShaderPass*, std::vector<std::size_t>> shaderIdxs;

		std::shared_mutex mu;
	private:
		auto BuildPipeline(const VulkanRenderPass& renderPass, const VulkanShaderPass& shader, Mesh::Topology topology) -> std::unique_ptr<VulkanPipeline>;
		auto ConvertStencilState(const StencilState& stencilState) const -> VkStencilOpState;
	public:
		SH_RENDER_API VulkanPipelineManager(const VulkanContext& context);
		SH_RENDER_API VulkanPipelineManager(VulkanPipelineManager&& other) noexcept;
		/// @brief 파이프라인을 생성하거나 가져온다. 스레드 안전하다.
		/// @param pass 렌더 패스
		/// @param shader 셰이더
		/// @param topology 메쉬 토폴로지
		/// @return 파이프라인 핸들
		SH_RENDER_API auto GetOrCreatePipelineHandle(const VulkanRenderPass& renderPass, const VulkanShaderPass& shader, Mesh::Topology topology) -> uint64_t;

		/// @brief vkCmdBindPipeline()의 래퍼 함수. 스레드 안전하다.
		/// @param cmd 커맨드 버퍼
		/// @param handle 파이프라인 핸들
		SH_RENDER_API bool BindPipeline(VkCommandBuffer cmd, uint64_t handle);
	};
}//namespace