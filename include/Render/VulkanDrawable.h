#pragma once

#include "Export.h"
#include "IDrawable.h"
#include "VulkanImpl/VulkanPipeline.h"

#include <memory>

namespace sh::render
{
	class VulkanRenderer;

	class VulkanDrawable : public IDrawable
	{
	private:
		std::unique_ptr<impl::VulkanPipeline> pipeline;

		VkDeviceMemory vertexBufferMem;
		VkBuffer vertexBuffer;

		const VulkanRenderer& renderer;
	public:
		SH_RENDER_API VulkanDrawable(const VulkanRenderer& renderer);
		SH_RENDER_API ~VulkanDrawable();

		SH_RENDER_API auto GetPipeline() const->impl::VulkanPipeline*;

		SH_RENDER_API auto GetVertexBuffer() const -> VkBuffer;
		SH_RENDER_API void Build(Material* mat, Mesh* mesh) override;
	};
}