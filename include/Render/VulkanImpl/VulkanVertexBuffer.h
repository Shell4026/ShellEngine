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

namespace sh::render
{
	class VulkanRenderer;

	class VulkanVertexBuffer : public IVertexBuffer
	{
	private:
		const VulkanRenderer& renderer;

		impl::VulkanCommandBuffer cmd;

		std::vector<VkVertexInputBindingDescription> mBindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> mAttribDescriptions;

		std::vector<impl::VulkanBuffer> buffers;
		impl::VulkanBuffer indexBuffer;
	private:
		inline void CreateVertexBuffer(const Mesh& mesh);
		inline void CreateAttributeBuffers(const Mesh& mesh);
	public:
		const std::vector<VkVertexInputBindingDescription>& bindingDescriptions;
		const std::vector<VkVertexInputAttributeDescription>& attribDescriptions;
	public:
		VulkanVertexBuffer(const VulkanRenderer& renderer);
		~VulkanVertexBuffer();

		SH_RENDER_API void Create(const Mesh& mesh) override;
		SH_RENDER_API void Clean() override;

		SH_RENDER_API void Bind() override;
	};
}