#pragma once
#include "Export.h"
#include "IDrawable.h"
#include "Camera.h"

#include "VulkanVertexBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"
#include "VulkanUniformBuffer.h"

#include <vector>
#include <memory>
#include <array>

namespace sh::render::vk
{
	class VulkanContext;

	class VulkanDrawable : public IDrawable
	{
		SCLASS(VulkanDrawable)
	private:
		const VulkanContext& context;
		PROPERTY(mat)
		Material* mat;
		PROPERTY(mesh)
		Mesh* mesh;
		Camera* camera;

		uint64_t pipelineHandle = 0;

		core::SyncArray<core::SMap<uint32_t, std::unique_ptr<VulkanBuffer>>> localVertBuffer;
		core::SyncArray<core::SMap<uint32_t, std::unique_ptr<VulkanBuffer>>> localFragBuffer;
		core::SyncArray<std::unique_ptr<VulkanUniformBuffer>> localDescSet;

		VulkanPipeline::Topology topology = VulkanPipeline::Topology::Triangle;

		bool bInit, bDirty, bBufferDirty;
	protected:
		void Clean(core::ThreadType thr);
		void CreateBuffers(core::ThreadType thr);
		void GetPipelineFromManager();
	public:
		SH_RENDER_API VulkanDrawable(const VulkanContext& context);
		SH_RENDER_API VulkanDrawable(VulkanDrawable&& other) noexcept;
		SH_RENDER_API VulkanDrawable(const VulkanDrawable& other) = delete;
		SH_RENDER_API ~VulkanDrawable() noexcept;

		SH_RENDER_API void Build(Camera& camera, Mesh* mesh, Material* mat) override;

		/// @brief [게임 스레드용] 로컬 유니폼에 데이터를 지정한다.
		/// @param binding 바인딩 번호
		/// @param data 데이터 위치 포인터
		/// @param stage 셰이더 스테이지
		SH_RENDER_API void SetUniformData(uint32_t binding, const void* data, Stage stage) override;

		SH_RENDER_API auto GetMaterial() const -> const Material* override;
		SH_RENDER_API auto GetMesh() const-> const Mesh* override;
		SH_RENDER_API auto GetCamera() const-> Camera* override;

		SH_RENDER_API auto GetPipelineHandle() const -> uint64_t;

		SH_RENDER_API auto GetLocalUniformBuffer(core::ThreadType thr) const -> VulkanUniformBuffer*;
		SH_RENDER_API auto GetDescriptorSet(core::ThreadType thr) const -> VkDescriptorSet;

		SH_RENDER_API void SetDirty() override;
		SH_RENDER_API void Sync() override;

		SH_RENDER_API bool CheckAssetValid() const override;
	};
}