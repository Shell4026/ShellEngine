
#include "Export.h"
#include "IDrawable.h"
#include "Camera.h"

#include "VulkanVertexBuffer.h"
#include "VulkanRenderer.h"
#include "VulkanPipeline.h"
#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"
#include "VulkanUniformBuffer.h"

#include <vector>
#include <memory>
#include <array>

namespace sh::render
{
	class Framebuffer;
	class Shader;

	class VulkanDrawable : public IDrawable
	{
		SCLASS(VulkanDrawable)
	private:
		VulkanRenderer& renderer;
		PROPERTY(mat)
		const Material* mat;
		PROPERTY(mesh)
		const Mesh* mesh;
		Camera* camera;

		//동기화 필요 목록
		core::SyncArray<std::unique_ptr<impl::VulkanPipeline>> pipeline;
		core::SyncArray<core::SMap<uint32_t, std::unique_ptr<impl::VulkanBuffer>>> localVertBuffer;
		core::SyncArray<core::SMap<uint32_t, std::unique_ptr<impl::VulkanBuffer>>> localFragBuffer;
		core::SyncArray<std::unique_ptr<impl::VulkanUniformBuffer>> localDescSet;

		impl::VulkanPipeline::Topology topology = impl::VulkanPipeline::Topology::Triangle;

		bool bInit, bDirty, bBufferDirty, bPipelineDirty;
	protected:
		void Clean(core::ThreadType thr);
		void CreateBuffer(core::ThreadType thr);
		void BuildPipeline(core::ThreadType thr, impl::VulkanPipeline::Topology topology);
	public:
		SH_RENDER_API VulkanDrawable(VulkanRenderer& renderer);
		SH_RENDER_API VulkanDrawable(VulkanDrawable&& other) noexcept;
		SH_RENDER_API VulkanDrawable(const VulkanDrawable& other) = delete;
		SH_RENDER_API ~VulkanDrawable() noexcept;

		SH_RENDER_API void Build(Camera& camera, const Mesh* mesh, const Material* mat) override;

		/// @brief [게임 스레드용] 로컬 유니폼에 데이터를 지정한다.
		/// @param binding 바인딩 번호
		/// @param data 데이터 위치 포인터
		/// @param stage 셰이더 스테이지
		SH_RENDER_API void SetUniformData(uint32_t binding, const void* data, Stage stage) override;

		SH_RENDER_API auto GetMaterial() const -> const Material* override;
		SH_RENDER_API auto GetMesh() const-> const Mesh* override;
		SH_RENDER_API auto GetCamera() const-> Camera* override;

		SH_RENDER_API auto GetPipeline(core::ThreadType thr) const->impl::VulkanPipeline*;

		SH_RENDER_API auto GetLocalUniformBuffer(core::ThreadType thr) const -> impl::VulkanUniformBuffer*;
		SH_RENDER_API auto GetDescriptorSet(core::ThreadType thr) const -> VkDescriptorSet;

		SH_RENDER_API void SetDirty() override;
		SH_RENDER_API void Sync() override;
	};
}