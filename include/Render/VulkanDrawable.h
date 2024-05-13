#pragma once

#include "Export.h"
#include "IDrawable.h"
#include "VulkanImpl/VulkanPipeline.h"
#include "VulkanImpl/VulkanBuffer.h"

#include <memory>

namespace sh::render
{
	class VulkanRenderer;

	class VulkanDrawable : public IDrawable
	{
	private:
		std::unique_ptr<impl::VulkanPipeline> pipeline;

		impl::VulkanBuffer vertexBuffer;

		const VulkanRenderer& renderer;

		Material* mat;
		Mesh* mesh;
	public:
		SH_RENDER_API VulkanDrawable(const VulkanRenderer& renderer);
		SH_RENDER_API ~VulkanDrawable();

		SH_RENDER_API auto GetPipeline() const->impl::VulkanPipeline*;

		SH_RENDER_API auto GetVertexBuffer() const -> const impl::VulkanBuffer&;
		SH_RENDER_API void Build(Material* mat, Mesh* mesh) override;
		SH_RENDER_API void Update() override;
	};
}