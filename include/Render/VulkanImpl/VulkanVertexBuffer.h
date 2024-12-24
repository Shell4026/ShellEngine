#pragma once

#include "../Export.h"
#include "../IVertexBuffer.h"
#include "../ShaderAttribute.h"

#include "VulkanConfig.h"
#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"

#include <cstdint>
#include <vector>
#include <string>
#include <string_view>

namespace sh::render::vk
{
	class VulkanRenderer;

	class VulkanVertexBuffer : public IVertexBuffer
	{
	private:
		const VulkanRenderer& renderer;

		VulkanCommandBuffer cmd;

		std::vector<VkVertexInputBindingDescription> mBindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> mAttribDescriptions;

		std::vector<VulkanBuffer> buffers;
		VulkanBuffer indexBuffer;
	private:
		inline void CreateVertexBuffer(const Mesh& mesh);
		inline void CreateAttributeBuffers(const Mesh& mesh);
	public:
		const std::vector<VkVertexInputBindingDescription>& bindingDescriptions;
		const std::vector<VkVertexInputAttributeDescription>& attribDescriptions;
	public:
		SH_RENDER_API VulkanVertexBuffer(const VulkanRenderer& renderer);
		SH_RENDER_API VulkanVertexBuffer(const VulkanVertexBuffer& other);
		SH_RENDER_API VulkanVertexBuffer(VulkanVertexBuffer&& other) noexcept;
		SH_RENDER_API ~VulkanVertexBuffer();

		SH_RENDER_API auto operator=(const VulkanVertexBuffer& other) -> VulkanVertexBuffer&;
		SH_RENDER_API auto operator=(VulkanVertexBuffer&& other) noexcept -> VulkanVertexBuffer&;

		SH_RENDER_API void Create(const Mesh& mesh) override;
		SH_RENDER_API void Clean() override;

		SH_RENDER_API void Bind() override;

		SH_RENDER_API auto Clone() const -> std::unique_ptr<IVertexBuffer> override;
	};
}