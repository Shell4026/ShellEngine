#include "VulkanVertexBuffer.h"
#include "VulkanContext.h"
#include "VulkanQueueManager.h"
#include "Mesh.h"

#include <cstddef>
#include <array>

#include "../vma-src/include/vk_mem_alloc.h"

namespace sh::render::vk
{
	VulkanVertexBuffer::VulkanVertexBuffer(const VulkanContext& context) :
		context(context),
		vertexBuffer(context),
		indexBuffer(context),
		cmd(context.GetDevice(), context.GetCommandPool(core::ThreadType::Game), VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT)
	{
	}
	VulkanVertexBuffer::VulkanVertexBuffer(const VulkanVertexBuffer& other) :
		context(other.context),
		vertexBuffer(other.vertexBuffer),
		indexBuffer(other.indexBuffer),
		cmd(context.GetDevice(), context.GetCommandPool(core::ThreadType::Game), VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT)
	{
	}
	VulkanVertexBuffer::VulkanVertexBuffer(VulkanVertexBuffer&& other) noexcept :
		context(other.context),
		vertexBuffer(std::move(other.vertexBuffer)),
		indexBuffer(std::move(other.indexBuffer)),
		cmd(std::move(other.cmd))
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
		cmd.Clear();

		vertexBuffer.Clean();
		indexBuffer.Clean();
	}

	void VulkanVertexBuffer::CreateVertexBuffer(const Mesh& mesh)
	{
		if (mesh.GetVertexCount() == 0)
			return;

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

		// 스테이징 버퍼에서 실제 버퍼로 복사
		VkCommandBufferBeginInfo info{};
		info.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		cmd.Build([&]()
		{
			VkBufferCopy cpy{};
			cpy.srcOffset = 0;
			cpy.dstOffset = 0;
			cpy.size = vertexBufferSize;
			vkCmdCopyBuffer(cmd.GetCommandBuffer(), stagingBuffer1.GetBuffer(), vertexBuffer.GetBuffer(), 1, &cpy);

			VkBufferCopy cpyIndices{};
			cpyIndices.srcOffset = 0;
			cpyIndices.dstOffset = 0;
			cpyIndices.size = indicesSize;
			vkCmdCopyBuffer(cmd.GetCommandBuffer(), stagingBuffer2.GetBuffer(), indexBuffer.GetBuffer(), 1, &cpyIndices);
		}, &info);

		context.GetQueueManager().SubmitCommand(cmd);
	}

	void VulkanVertexBuffer::Create(const Mesh& mesh)
	{
		Clean();

		cmd.Create();
		CreateVertexBuffer(mesh);
	}

	void VulkanVertexBuffer::Bind()
	{
		std::array<VkBuffer, 1> buffers = { vertexBuffer.GetBuffer() };

		assert(vertexBuffer.GetBuffer() != VK_NULL_HANDLE);
		assert(indexBuffer.GetBuffer() != VK_NULL_HANDLE);
		assert(context.GetCommandBuffer(core::ThreadType::Render)->GetCommandBuffer() != VK_NULL_HANDLE);
		
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(context.GetCommandBuffer(core::ThreadType::Render)->GetCommandBuffer(), 0, 1, buffers.data(), offsets);
		vkCmdBindIndexBuffer(context.GetCommandBuffer(core::ThreadType::Render)->GetCommandBuffer(), indexBuffer.GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
	}

	auto VulkanVertexBuffer::Clone() const -> std::unique_ptr<IVertexBuffer>
	{
		return std::make_unique<VulkanVertexBuffer>(*this);
	}
	SH_RENDER_API auto sh::render::vk::VulkanVertexBuffer::GetBindingDescription() -> VkVertexInputBindingDescription
	{
		static VkVertexInputBindingDescription bindingDescription;
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Mesh::Vertex);
		bindingDescription.inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}
	SH_RENDER_API auto sh::render::vk::VulkanVertexBuffer::GetAttributeDescriptions() -> std::vector<VkVertexInputAttributeDescription>
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
}