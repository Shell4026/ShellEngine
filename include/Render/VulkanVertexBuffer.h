#pragma once

#include "Export.h"
#include "VertexBuffer.h"
#include "ShaderAttribute.h"

#include "VulkanImpl/VulkanConfig.h"
#include "VulkanImpl/VulkanBuffer.h"
#include "VulkanImpl/VulkanCommandBuffer.h"

#include <cstdint>
#include <vector>
#include <string>
#include <string_view>

namespace sh::render
{
	class VulkanRenderer;

	class VulkanVertexBuffer : public VertexBuffer
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

		void Create(const Mesh& mesh) override;
		void Clean() override;

		void Bind() override;
	};
}