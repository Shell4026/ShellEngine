#pragma once

#include "Export.h"
#include "IDrawable.h"
#include "VulkanImpl/VulkanPipeline.h"
#include "VulkanImpl/VulkanBuffer.h"
#include "VulkanImpl/VulkanCommandBuffer.h"

#include <vector>
#include <memory>

namespace sh::render
{
	class VulkanRenderer;

	class VulkanDrawable : public IDrawable
	{
	private:
		const VulkanRenderer& renderer;
		Material* mat;
		Mesh* mesh;

		VkPipelineLayout pipelineLayout;
		std::unique_ptr<impl::VulkanPipeline> pipeline;

		std::vector<impl::VulkanBuffer> vertexBuffers;
		impl::VulkanBuffer indexBuffer;

		impl::VulkanCommandBuffer cmd;

		VkDescriptorSetLayout descriptorSetLayout;
		std::vector<VkDescriptorSet> descriptorSets;
		std::vector<impl::VulkanBuffer> uniformBuffers;
	private:
		void CreateVertexBuffer();
		auto CreatePipelineLayout() -> VkResult;
		auto CreateDescriptorLayout(uint32_t binding) -> VkResult;
	public:
		const std::vector<impl::VulkanBuffer>& buffers;
	public:
		SH_RENDER_API VulkanDrawable(const VulkanRenderer& renderer);
		SH_RENDER_API ~VulkanDrawable();

		SH_RENDER_API auto GetPipelineLayout() const -> VkPipelineLayout;
		SH_RENDER_API auto GetPipeline() const->impl::VulkanPipeline*;

		SH_RENDER_API auto GetVertexBuffer() const -> const impl::VulkanBuffer&;
		SH_RENDER_API auto GetIndexBuffer() const -> const impl::VulkanBuffer&;
		SH_RENDER_API void Build(Material* mat, Mesh* mesh) override;

		SH_RENDER_API auto GetMaterial() const -> Material* override;
		SH_RENDER_API auto GetMesh() const-> Mesh* override;

		SH_RENDER_API void SetUniformData(int frame, const void* data);
		SH_RENDER_API auto GetDescriptorSet(int frame) -> VkDescriptorSet;
	};
}