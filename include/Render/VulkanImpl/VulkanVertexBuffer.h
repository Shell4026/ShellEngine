#pragma once
#include "Export.h"
#include "IVertexBuffer.h"
#include "VulkanBuffer.h"

#include <cstdint>
#include <vector>

namespace sh::render::vk
{
	class VulkanContext;

	class VulkanVertexBuffer : public IVertexBuffer
	{
	private:
		const VulkanContext& context;

		VulkanBuffer vertexBuffer;
		VulkanBuffer indexBuffer;

		SH_RENDER_API static inline std::vector<VkVertexInputAttributeDescription> attribDescriptions;
	private:
		inline void CreateVertexBuffer(const Mesh& mesh);
	public:
		SH_RENDER_API VulkanVertexBuffer(const VulkanContext& context);
		SH_RENDER_API VulkanVertexBuffer(const VulkanVertexBuffer& other);
		SH_RENDER_API VulkanVertexBuffer(VulkanVertexBuffer&& other) noexcept;
		SH_RENDER_API ~VulkanVertexBuffer();

		SH_RENDER_API auto operator=(const VulkanVertexBuffer& other) -> VulkanVertexBuffer&;
		SH_RENDER_API auto operator=(VulkanVertexBuffer&& other) noexcept -> VulkanVertexBuffer&;

		SH_RENDER_API void Create(const Mesh& mesh) override;
		SH_RENDER_API void Clean() override;

		SH_RENDER_API auto Clone() const -> std::unique_ptr<IVertexBuffer> override;

		SH_RENDER_API static auto GetBindingDescription() -> VkVertexInputBindingDescription;
		SH_RENDER_API static auto GetAttributeDescriptions() -> std::vector<VkVertexInputAttributeDescription>;

		SH_RENDER_API auto GetVertexBuffer() const -> const VulkanBuffer&;
		SH_RENDER_API auto GetIndexBuffer() const -> const VulkanBuffer&;
	};
}