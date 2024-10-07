
#include "Export.h"
#include "IDrawable.h"
#include "Camera.h"

#include "VulkanVertexBuffer.h"
#include "VulkanRenderer.h"
#include "VulkanPipeline.h"
#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"

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
		Material* mat;
		PROPERTY(mesh)
		Mesh* mesh;
		PROPERTY(textures)
		std::map<uint32_t, Texture*> textures;
		Camera* camera;

		
		//동기화 필요 목록
		core::SyncArray<std::unique_ptr<impl::VulkanPipeline>> pipeline;
		core::SyncArray<VkDescriptorSet> descriptorSet;
		core::SyncArray<std::map<uint32_t, impl::VulkanBuffer>> vertUniformBuffers;
		core::SyncArray<std::map<uint32_t, impl::VulkanBuffer>> fragUniformBuffers;

		bool bInit;
		bool bDirty;
		bool bTextureDirty;
		bool bPipelineDirty;
	private:
		void CreateUniformBuffers(core::ThreadType thr, const Shader& shader);
		void UpdateDescriptors(core::ThreadType thr);
	public:
		SH_RENDER_API VulkanDrawable(VulkanRenderer& renderer);
		SH_RENDER_API VulkanDrawable(VulkanDrawable&& other) noexcept;
		SH_RENDER_API VulkanDrawable(const VulkanDrawable& other) = delete;
		SH_RENDER_API ~VulkanDrawable() noexcept;

		SH_RENDER_API void Clean();

		SH_RENDER_API void Build(Camera& camera, Mesh& mesh, Material* mat) override;

		SH_RENDER_API auto GetMaterial() const -> Material* override;
		SH_RENDER_API auto GetMesh() const-> Mesh* override;
		SH_RENDER_API auto GetCamera() const-> Camera* override;

		/// @brief 유니폼에 데이터를 지정한다.
		/// @param binding 바인딩 번호
		/// @param data 데이터 위치 포인터
		/// @param stage 셰이더 스테이지
		/// @return 
		SH_RENDER_API void SetUniformData(uint32_t binding, const void* data, Stage stage) override;

		/// @brief 유니폼 텍스쳐 데이터를 지정한다.
		/// @param binding 텍스쳐 바인딩 번호
		/// @param tex 텍스쳐 포인터
		/// @return 
		SH_RENDER_API void SetTextureData(uint32_t binding, Texture* tex) override;

		SH_RENDER_API auto GetPipeline(core::ThreadType thr) const->impl::VulkanPipeline*;
		SH_RENDER_API auto GetDescriptorSet() const -> VkDescriptorSet;

		SH_RENDER_API void SetDirty() override;
		SH_RENDER_API void Sync() override;
	};
}