#pragma once

#include "Export.h"
#include "IDrawable.h"
#include "VulkanImpl/VulkanPipeline.h"
#include "VulkanImpl/VulkanBuffer.h"
#include "VulkanImpl/VulkanCommandBuffer.h"

#include <memory>

namespace sh::render
{
	class VulkanRenderer;

	class VulkanDrawable : public IDrawable
	{
	private:
		std::unique_ptr<impl::VulkanPipeline> pipeline;

		impl::VulkanBuffer indexBuffer;
		std::vector<impl::VulkanBuffer> vertexBuffers;
		impl::VulkanCommandBuffer cmd;

		const VulkanRenderer& renderer;

		Material* mat;
		Mesh* mesh;
	private:
		SH_RENDER_API void CreateVertexBuffer();
	public:
		const std::vector<impl::VulkanBuffer>& buffers;
	public:
		SH_RENDER_API VulkanDrawable(const VulkanRenderer& renderer);
		SH_RENDER_API ~VulkanDrawable();

		SH_RENDER_API auto GetPipeline() const->impl::VulkanPipeline*;

		SH_RENDER_API auto GetVertexBuffer() const -> const impl::VulkanBuffer&;
		SH_RENDER_API auto GetIndexBuffer() const -> const impl::VulkanBuffer&;
		SH_RENDER_API void Build(Material* mat, Mesh* mesh) override;
		SH_RENDER_API void Update() override;
	};
}