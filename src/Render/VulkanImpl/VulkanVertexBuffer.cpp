#include "VulkanVertexBuffer.h"
#include "VulkanContext.h"
#include "VulkanQueueManager.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommandBufferPool.h"
#include "Mesh.h"

#include <cstddef>
#include <array>

#include "../vma-src/include/vk_mem_alloc.h"

#include <limits>
namespace sh::render::vk
{
	VulkanVertexBuffer::VulkanVertexBuffer(const VulkanContext& context) :
		context(context),
		vertexBuffer(context),
		indexBuffer(context)
	{
	}
	VulkanVertexBuffer::VulkanVertexBuffer(const VulkanVertexBuffer& other) :
		context(other.context),
		vertexBuffer(other.vertexBuffer),
		indexBuffer(other.indexBuffer)
	{
	}
	VulkanVertexBuffer::VulkanVertexBuffer(VulkanVertexBuffer&& other) noexcept :
		context(other.context),
		vertexBuffer(std::move(other.vertexBuffer)),
		indexBuffer(std::move(other.indexBuffer))
	{
	}
	VulkanVertexBuffer::~VulkanVertexBuffer()
	{
		Clear();
	}
	SH_RENDER_API auto VulkanVertexBuffer::operator=(const VulkanVertexBuffer& other) -> VulkanVertexBuffer&
	{
		if (&other == this)
			return *this;
		vertexBuffer = other.vertexBuffer;
		indexBuffer = other.indexBuffer;

		return *this;
	}
	SH_RENDER_API auto VulkanVertexBuffer::operator=(VulkanVertexBuffer&& other) noexcept ->VulkanVertexBuffer&
	{
		if (&other == this)
			return *this;
		vertexBuffer = std::move(other.vertexBuffer);
		indexBuffer = std::move(other.indexBuffer);
		return *this;
	}
	SH_RENDER_API void VulkanVertexBuffer::Clear()
	{
		vertexBuffer.Clean();
		indexBuffer.Clean();
	}
	SH_RENDER_API void VulkanVertexBuffer::Create(const Mesh& mesh)
	{
		Clear();

		CreateVertexBuffer(mesh);
	}
	SH_RENDER_API auto VulkanVertexBuffer::Clone() const -> std::unique_ptr<IVertexBuffer>
	{
		return std::make_unique<VulkanVertexBuffer>(*this);
	}
	SH_RENDER_API auto VulkanVertexBuffer::GetBindingDescription() -> VkVertexInputBindingDescription
	{
		static VkVertexInputBindingDescription bindingDescription;
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Mesh::Vertex);
		bindingDescription.inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}
	SH_RENDER_API auto VulkanVertexBuffer::GetAttributeDescriptions() -> std::vector<VkVertexInputAttributeDescription>
	{
		if (attribDescriptions.empty())
		{
			VkVertexInputAttributeDescription attrDesc{};
			attrDesc.binding = 0;
			attrDesc.location = Mesh::VERTEX_ID;
			attrDesc.format = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
			attrDesc.offset = offsetof(Mesh::Vertex, vertex);
			attribDescriptions.push_back(attrDesc);
			attrDesc.location = Mesh::UV_ID;
			attrDesc.format = VkFormat::VK_FORMAT_R32G32_SFLOAT;
			attrDesc.offset = offsetof(Mesh::Vertex, uv);
			attribDescriptions.push_back(attrDesc);
			attrDesc.location = Mesh::NORMAL_ID;
			attrDesc.format = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
			attrDesc.offset = offsetof(Mesh::Vertex, normal);
			attribDescriptions.push_back(attrDesc);
			attrDesc.location = Mesh::TANGENT_ID;
			attrDesc.format = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
			attrDesc.offset = offsetof(Mesh::Vertex, tangent);
			attribDescriptions.push_back(attrDesc);
		}
		return attribDescriptions;
	}
	void VulkanVertexBuffer::CreateVertexBuffer(const Mesh& mesh)
	{
		if (mesh.GetVertexCount() == 0)
			return;

		// 버텍스 버퍼
		const size_t vertexBufferSize = sizeof(Mesh::Vertex) * mesh.GetVertexCount();
		vertexBuffer.Create(vertexBufferSize,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT | VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		vertexBuffer.SetData(mesh.GetVertex().data());

		// 인덱스 버퍼
		const size_t indicesSize = sizeof(uint32_t) * mesh.GetIndices().size();
		indexBuffer.Create(indicesSize,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT | VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		indexBuffer.SetData(mesh.GetIndices().data());
	}
}//namespace