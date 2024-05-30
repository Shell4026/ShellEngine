#pragma once

#include "Export.h"
#include "IDrawable.h"
#include "VulkanVertexBuffer.h"
#include "VulkanRenderer.h"

#include "VulkanImpl/VulkanPipeline.h"
#include "VulkanImpl/VulkanBuffer.h"
#include "VulkanImpl/VulkanCommandBuffer.h"

#include <vector>
#include <memory>

namespace sh::render
{
	class VulkanDrawable : public IDrawable
	{
	private:
		VulkanRenderer& renderer;
		Material* mat;
		Mesh* mesh;

		std::unique_ptr<impl::VulkanPipeline> pipeline;

		impl::VulkanCommandBuffer cmd;

		std::array<VkDescriptorSet, VulkanRenderer::MAX_FRAME_DRAW> descriptorSets;

		std::map<uint32_t, std::vector<impl::VulkanBuffer>> uniformBuffers;
		std::map<uint32_t, Texture*> textures;
	private:
		auto CreateDescriptorSet() -> VkResult;
	public:
		SH_RENDER_API VulkanDrawable(VulkanRenderer& renderer);
		SH_RENDER_API VulkanDrawable(const VulkanDrawable& other) = delete;
		SH_RENDER_API VulkanDrawable(VulkanDrawable&& other) = delete;
		SH_RENDER_API ~VulkanDrawable();

		SH_RENDER_API void Clean();

		SH_RENDER_API void Build(Mesh* mesh, Material* mat) override;

		SH_RENDER_API auto GetMaterial() const -> Material* override;
		SH_RENDER_API auto GetMesh() const-> Mesh* override;

		SH_RENDER_API void SetUniformData(uint32_t binding, int frame, const void* data) override;
		SH_RENDER_API void SetTextureData(uint32_t binding, Texture* tex) override;

		SH_RENDER_API auto GetPipeline() const->impl::VulkanPipeline*;
		SH_RENDER_API auto GetDescriptorSet(int frame) -> VkDescriptorSet;
	};
}