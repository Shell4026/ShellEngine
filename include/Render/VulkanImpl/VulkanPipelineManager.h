#pragma once
#include "Export.h"
#include "VulkanPipeline.h"

#include "Core/SContainer.hpp"
#include "Core/Util.h"
#include "Core/NonCopyable.h"
#include "Core/ISyncable.h"
#include "Core/Observer.hpp"

#include <memory>

namespace sh::render
{
	class VulkanShader;
	class Mesh;
}
namespace sh::render::vk
{
	class VulkanPipelineManager : public core::INonCopyable
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

		VkDevice device = nullptr;
		
		core::SVector<core::SyncArray<std::unique_ptr<VulkanPipeline>>> pipelines;
		core::SVector<PipelineInfo> pipelinesInfo;
		std::unordered_map<PipelineInfo, std::size_t, PipelineInfoHasher> infoIdx;
		std::unordered_map<const VkRenderPass*, core::SVector<std::size_t>> renderpassIdxs;
		std::unordered_map<const VulkanShader*, core::SVector<std::size_t>> shaderIdxs;
		std::unordered_map<const Mesh*, core::SVector<std::size_t>> meshIdxs;

		core::Observer<false, core::SObject*>::Listener shaderListener;
		core::Observer<false, core::SObject*>::Listener meshListener;
	private:
		auto BuildPipeline(const VkRenderPass& pass, VulkanShader& shader, Mesh& mesh) -> std::unique_ptr<VulkanPipeline>;
	public:
		VulkanPipelineManager(VkDevice device);
		/// @brief 파이프라인을 생성하거나 가져온다.
		/// @param thr 스레드
		/// @param pass 렌더 패스
		/// @param mesh 메쉬
		/// @param shader 셰이더
		/// @return 파이프라인
		SH_RENDER_API auto GetPipeline(core::ThreadType thr, const VkRenderPass& pass, VulkanShader& shader, Mesh& mesh) -> VulkanPipeline*;
	};
}//namespace