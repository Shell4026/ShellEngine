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
		Clean();
	}
	auto VulkanVertexBuffer::operator=(const VulkanVertexBuffer& other) -> VulkanVertexBuffer&
	{
		vertexBuffer = other.vertexBuffer;
		indexBuffer = other.indexBuffer;

		return *this;
	}
	auto VulkanVertexBuffer::operator=(VulkanVertexBuffer&& other) noexcept ->VulkanVertexBuffer&
	{
		vertexBuffer = std::move(other.vertexBuffer);
		indexBuffer = std::move(other.indexBuffer);
		return *this;
	}

	void VulkanVertexBuffer::Clean()
	{
		vertexBuffer.Clean();
		indexBuffer.Clean();
	}

	void VulkanVertexBuffer::CreateVertexBuffer(const Mesh& mesh)
	{
		if (mesh.GetVertexCount() == 0)
			return;

		VulkanCommandBuffer* cmd = context.GetCommandBufferPool().AllocateCommandBuffer(std::this_thread::get_id(), VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT);
		assert(cmd != nullptr);
		// 버텍스 버퍼
		size_t vertexBufferSize = sizeof(Mesh::Vertex) * mesh.GetVertexCount();
		VulkanBuffer stagingBuffer1{ context };
		stagingBuffer1.Create(vertexBufferSize, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		stagingBuffer1.SetData(mesh.GetVertex().data());

		vertexBuffer.Create(vertexBufferSize,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT | VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		// 인덱스 버퍼
		size_t indicesSize = sizeof(uint32_t) * mesh.GetIndices().size();
		VulkanBuffer stagingBuffer2{ context };
		stagingBuffer2.Create(indicesSize, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		stagingBuffer2.SetData(mesh.GetIndices().data());

		indexBuffer.Create(indicesSize,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT | VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		cmd->Build(
			[&]()
			{
				VkBufferCopy cpy{};
				cpy.srcOffset = 0;
				cpy.dstOffset = 0;
				cpy.size = vertexBufferSize;
				vkCmdCopyBuffer(cmd->GetCommandBuffer(), stagingBuffer1.GetBuffer(), vertexBuffer.GetBuffer(), 1, &cpy);

				VkBufferCopy cpyIndices{};
				cpyIndices.srcOffset = 0;
				cpyIndices.dstOffset = 0;
				cpyIndices.size = indicesSize;
				vkCmdCopyBuffer(cmd->GetCommandBuffer(), stagingBuffer2.GetBuffer(), indexBuffer.GetBuffer(), 1, &cpyIndices);
			}, 
			true
		);

		VkQueue transferQueue = context.GetQueueManager().GetTransferQueue();

		VkFence fence = cmd->GetFence();
		context.GetQueueManager().SubmitCommand(transferQueue, *cmd, fence);
		vkWaitForFences(context.GetDevice(), 1, &fence, true, std::numeric_limits<uint64_t>::max());
		vkResetFences(context.GetDevice(), 1, &fence);

		context.GetCommandBufferPool().DeallocateCommandBuffer(*cmd);
	}

	void VulkanVertexBuffer::Create(const Mesh& mesh)
	{
		Clean();

		CreateVertexBuffer(mesh);
	}
	auto VulkanVertexBuffer::Clone() const -> std::unique_ptr<IVertexBuffer>
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

	SH_RENDER_API auto VulkanVertexBuffer::GetVertexBuffer() const -> const VulkanBuffer&
	{
		return vertexBuffer;
	}
	SH_RENDER_API auto VulkanVertexBuffer::GetIndexBuffer() const -> const VulkanBuffer&
	{
		return indexBuffer;
	}
}