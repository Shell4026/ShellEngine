#include "VulkanSkinnedVertexBuffer.h"
#include "VulkanContext.h"
#include "VulkanQueueManager.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommandBufferPool.h"
#include "Mesh.h"
#include "SkinnedMesh.h"

#include <cstddef>
#include <array>
#include <cassert>
#include <limits>

namespace sh::render::vk
{
	VulkanSkinnedVertexBuffer::VulkanSkinnedVertexBuffer(const VulkanContext& context) :
		context(context),
		vertexBuffer(context),
		boneBuffer(context),
		indexBuffer(context)
	{
	}
	VulkanSkinnedVertexBuffer::VulkanSkinnedVertexBuffer(const VulkanSkinnedVertexBuffer& other) :
		context(other.context),
		vertexBuffer(other.vertexBuffer),
		boneBuffer(other.boneBuffer),
		indexBuffer(other.indexBuffer)
	{
	}
	VulkanSkinnedVertexBuffer::VulkanSkinnedVertexBuffer(VulkanSkinnedVertexBuffer&& other) noexcept :
		context(other.context),
		vertexBuffer(std::move(other.vertexBuffer)),
		boneBuffer(std::move(other.boneBuffer)),
		indexBuffer(std::move(other.indexBuffer))
	{
	}
	VulkanSkinnedVertexBuffer::~VulkanSkinnedVertexBuffer()
	{
		Clear();
	}
	SH_RENDER_API auto VulkanSkinnedVertexBuffer::operator=(const VulkanSkinnedVertexBuffer& other) -> VulkanSkinnedVertexBuffer&
	{
		if (&other == this)
			return *this;
		vertexBuffer = other.vertexBuffer;
		boneBuffer = other.boneBuffer;
		indexBuffer = other.indexBuffer;
		return *this;
	}
	SH_RENDER_API auto VulkanSkinnedVertexBuffer::operator=(VulkanSkinnedVertexBuffer&& other) noexcept -> VulkanSkinnedVertexBuffer&
	{
		if (&other == this)
			return *this;
		vertexBuffer = std::move(other.vertexBuffer);
		boneBuffer = std::move(other.boneBuffer);
		indexBuffer = std::move(other.indexBuffer);
		return *this;
	}
	SH_RENDER_API void VulkanSkinnedVertexBuffer::Clear()
	{
		vertexBuffer.Clean();
		boneBuffer.Clean();
		indexBuffer.Clean();
	}
	SH_RENDER_API void VulkanSkinnedVertexBuffer::Create(const Mesh& mesh)
	{
		Clear();
		CreateBuffers(mesh);
	}
	SH_RENDER_API auto VulkanSkinnedVertexBuffer::Clone() const -> std::unique_ptr<IVertexBuffer>
	{
		return std::make_unique<VulkanSkinnedVertexBuffer>(*this);
	}
	SH_RENDER_API auto VulkanSkinnedVertexBuffer::GetBindingDescriptions() -> std::array<VkVertexInputBindingDescription, 2>
	{
		std::array<VkVertexInputBindingDescription, 2> bindings{};
		// binding 0: Mesh::Vertex
		bindings[0].binding = 0;
		bindings[0].stride = sizeof(Mesh::Vertex);
		bindings[0].inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;
		// binding 1: SkinnedMesh::BoneVertex
		bindings[1].binding = 1;
		bindings[1].stride = sizeof(SkinnedMesh::BoneVertex);
		bindings[1].inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;
		return bindings;
	}
	SH_RENDER_API auto VulkanSkinnedVertexBuffer::GetAttributeDescriptions() -> std::vector<VkVertexInputAttributeDescription>
	{
		if (attribDescriptions.empty())
		{
			VkVertexInputAttributeDescription attrDesc{};

			// binding 0: Mesh::Vertex 속성 (location 0-3)
			attrDesc.binding = 0;
			attrDesc.location = Mesh::VERTEX_ID; // 0
			attrDesc.format = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
			attrDesc.offset = offsetof(Mesh::Vertex, vertex);
			attribDescriptions.push_back(attrDesc);

			attrDesc.location = Mesh::UV_ID; // 1
			attrDesc.format = VkFormat::VK_FORMAT_R32G32_SFLOAT;
			attrDesc.offset = offsetof(Mesh::Vertex, uv);
			attribDescriptions.push_back(attrDesc);

			attrDesc.location = Mesh::NORMAL_ID; // 2
			attrDesc.format = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
			attrDesc.offset = offsetof(Mesh::Vertex, normal);
			attribDescriptions.push_back(attrDesc);

			attrDesc.location = Mesh::TANGENT_ID; // 3
			attrDesc.format = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
			attrDesc.offset = offsetof(Mesh::Vertex, tangent);
			attribDescriptions.push_back(attrDesc);

			// binding 1: BoneVertex 속성 (location 4-5)
			attrDesc.binding = 1;
			attrDesc.location = SkinnedMesh::BONE_INDEX_ID; // 4
			attrDesc.format = VkFormat::VK_FORMAT_R32G32B32A32_SINT;
			attrDesc.offset = offsetof(SkinnedMesh::BoneVertex, boneIndices);
			attribDescriptions.push_back(attrDesc);

			attrDesc.location = SkinnedMesh::BONE_WEIGHT_ID; // 5
			attrDesc.format = VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT;
			attrDesc.offset = offsetof(SkinnedMesh::BoneVertex, boneWeights);
			attribDescriptions.push_back(attrDesc);
		}
		return attribDescriptions;
	}

	void VulkanSkinnedVertexBuffer::CreateBuffers(const Mesh& mesh)
	{
		if (mesh.GetVertexCount() == 0)
			return;

		const SkinnedMesh* skinnedMesh = static_cast<const SkinnedMesh*>(&mesh);

		VulkanCommandBuffer* cmd = context.GetCommandBufferPool().AllocateCommandBuffer(
			std::this_thread::get_id(), VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT);
		assert(cmd != nullptr);

		// Mesh::Vertex 버퍼
		size_t vertexBufferSize = sizeof(Mesh::Vertex) * mesh.GetVertexCount();
		VulkanBuffer stagingVert{ context };
		stagingVert.Create(vertexBufferSize,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		stagingVert.SetData(mesh.GetVertex().data());

		vertexBuffer.Create(vertexBufferSize,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT | VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		// BoneVertex 버퍼
		const auto& boneVerts = skinnedMesh->GetBoneVertices();
		size_t boneBufferSize = sizeof(SkinnedMesh::BoneVertex) * boneVerts.size();
		VulkanBuffer stagingBone{ context };
		if (boneBufferSize > 0)
		{
			stagingBone.Create(boneBufferSize,
				VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
				VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			stagingBone.SetData(boneVerts.data());

			boneBuffer.Create(boneBufferSize,
				VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT | VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
				VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		}

		// Index 버퍼
		size_t indicesSize = sizeof(uint32_t) * mesh.GetIndices().size();
		VulkanBuffer stagingIdx{ context };
		stagingIdx.Create(indicesSize,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		stagingIdx.SetData(mesh.GetIndices().data());

		indexBuffer.Create(indicesSize,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT | VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		// 단일 커맨드로 모두 전송
		cmd->Build(
			[&]()
			{
				VkBufferCopy cpy{};
				cpy.size = vertexBufferSize;
				vkCmdCopyBuffer(cmd->GetCommandBuffer(), stagingVert.GetBuffer(), vertexBuffer.GetBuffer(), 1, &cpy);

				if (boneBufferSize > 0)
				{
					cpy.size = boneBufferSize;
					vkCmdCopyBuffer(cmd->GetCommandBuffer(), stagingBone.GetBuffer(), boneBuffer.GetBuffer(), 1, &cpy);
				}

				cpy.size = indicesSize;
				vkCmdCopyBuffer(cmd->GetCommandBuffer(), stagingIdx.GetBuffer(), indexBuffer.GetBuffer(), 1, &cpy);
			},
			true
		);

		VkFence fence = cmd->GetOrCreateFence();
		context.GetQueueManager().Submit(VulkanQueueManager::Role::Transfer, *cmd, fence);
		vkWaitForFences(context.GetDevice(), 1, &fence, true, std::numeric_limits<uint64_t>::max());
		vkResetFences(context.GetDevice(), 1, &fence);

		context.GetCommandBufferPool().DeallocateCommandBuffer(*cmd);
	}
}//namespace
