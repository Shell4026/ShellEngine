#pragma once
#include "Export.h"
#include "VulkanPipeline.h"

#include "Core/SContainer.hpp"
#include "Core/Util.h"
#include "Core/NonCopyable.h"
#include "Core/ISyncable.h"
#include "Core/Observer.hpp"

#include <memory>
#include <stack>

namespace sh::render
{
	class VulkanShader;
	class Mesh;
}
namespace sh::render::vk
{
	class VulkanContext;

	class VulkanPipelineManager : public core::INonCopyable, public core::ISyncable
	{
	private:
		struct PipelineInfo
		{
			const VkRenderPass* pass;
			const VulkanShader* shader;
			const Mesh* mesh;

			bool operator==(const PipelineInfo& other) const
			{
				return pass == other.pass && shader == other.shader && mesh == other.mesh;
			}
		};
		struct PipelineInfoHasher
		{
			auto operator()(const VulkanPipelineManager::PipelineInfo& info) const -> std::size_t
			{
				using namespace sh::core;
				std::hash<const void*> hasher;
				std::size_t hash0 = hasher(info.pass);
				std::size_t hash1 = hasher(info.shader);
				std::size_t hash2 = hasher(info.mesh);
				return Util::CombineHash(Util::CombineHash(hash0, hash1), hash2);
			}
		};
		const VulkanContext& context;
		VkDevice device = nullptr;
		
		core::SVector<core::SyncArray<std::unique_ptr<VulkanPipeline>>> pipelines;
		core::SVector<PipelineInfo> pipelinesInfo;
		std::unordered_map<PipelineInfo, std::size_t, PipelineInfoHasher> infoIdx;
		std::unordered_map<const VkRenderPass*, core::SVector<std::size_t>> renderpassIdxs;
		std::unordered_map<const VulkanShader*, core::SVector<std::size_t>> shaderIdxs;
		std::unordered_map<const Mesh*, core::SVector<std::size_t>> meshIdxs;

		core::Observer<false, core::SObject*>::Listener shaderListener;
		core::Observer<false, core::SObject*>::Listener meshListener;

		VulkanPipeline* lastBindingPipeline = nullptr;

		bool bDirty = false;
		std::stack<uint64_t> dirtyPipelines;
	private:
		auto BuildPipeline(const VkRenderPass& pass, VulkanShader& shader, Mesh& mesh) -> std::unique_ptr<VulkanPipeline>;
	public:
		VulkanPipelineManager(const VulkanContext& context);
		/// @brief 파이프라인을 생성하거나 가져온다.
		/// @param thr 스레드
		/// @param pass 렌더 패스
		/// @param mesh 메쉬
		/// @param shader 셰이더
		/// @return 파이프라인 핸들
		SH_RENDER_API auto GetPipelineHandle(const VkRenderPass& pass, VulkanShader& shader, Mesh& mesh) -> uint64_t;

		/// @brief 매 렌더링 시작 할 때 호출해야 하는 함수
		SH_RENDER_API void BeginRender();
		SH_RENDER_API bool BindPipeline(VkCommandBuffer cmd, uint64_t handle);

		SH_RENDER_API void SetDirty() override;
		SH_RENDER_API void Sync() override;
	};
}//namespace