#include "VulkanVertexBuffer.h"

#include "VulkanRenderer.h"
#include "Mesh.h"
#include "../vma-src/include/vk_mem_alloc.h"

namespace sh::render
{
	VulkanVertexBuffer::VulkanVertexBuffer(const VulkanRenderer& renderer) :
		renderer(renderer),
		bindingDescriptions(mBindingDescriptions),
		attribDescriptions(mAttribDescriptions),

		indexBuffer(renderer.GetDevice(), renderer.GetGPU(), renderer.GetAllocator()),
		cmd(renderer.GetDevice(), renderer.GetCommandPool())
	{
	}

	VulkanVertexBuffer::~VulkanVertexBuffer()
	{
		Clean();
	}

	void VulkanVertexBuffer::Clean()
	{
		cmd.Clean();

		mBindingDescriptions.clear();
		mAttribDescriptions.clear();

		buffers.clear();
		indexBuffer.Clean();
	}

	void VulkanVertexBuffer::CreateVertexBuffer(const Mesh& mesh)
	{
		VkVertexInputBindingDescription bindingDesc{};
		bindingDesc.binding = 0;
		bindingDesc.stride = sizeof(glm::vec3);
		bindingDesc.inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;
		mBindingDescriptions.push_back(bindingDesc);

		VkVertexInputAttributeDescription attrDesc{};
		attrDesc.binding = 0;
		attrDesc.location = 0;
		attrDesc.format = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
		attrDesc.offset = 0;
		mAttribDescriptions.push_back(attrDesc);

		size_t size = sizeof(glm::vec3) * mesh.GetVertexCount();
		impl::VulkanBuffer stagingBuffer1{ renderer.GetDevice(), renderer.GetGPU(), renderer.GetAllocator() };
		stagingBuffer1.Create(size, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		stagingBuffer1.SetData(mesh.GetVertex().data());

		buffers.push_back(impl::VulkanBuffer{ renderer.GetDevice(), renderer.GetGPU(), renderer.GetAllocator() });
		buffers[0].Create(size,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		size_t sizeIndices = sizeof(uint32_t) * mesh.GetIndices().size();
		impl::VulkanBuffer stagingBuffer2{ renderer.GetDevice(), renderer.GetGPU(), renderer.GetAllocator() };
		stagingBuffer2.Create(sizeIndices, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		stagingBuffer2.SetData(mesh.GetIndices().data());

		indexBuffer.Create(sizeIndices,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkCommandBufferBeginInfo info{};
		info.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		cmd.Submit(renderer.GetGraphicsQueue(), [&]() {
			VkBufferCopy cpy{};
			cpy.srcOffset = 0; // Optional
			cpy.dstOffset = 0; // Optional
			cpy.size = size;

			VkBufferCopy cpyIndices{};
			cpyIndices.srcOffset = 0; // Optional
			cpyIndices.dstOffset = 0; // Optional
			cpyIndices.size = sizeIndices;
			vkCmdCopyBuffer(cmd.GetCommandBuffer(), stagingBuffer1.GetBuffer(), buffers[0].GetBuffer(), 1, &cpy);
			vkCmdCopyBuffer(cmd.GetCommandBuffer(), stagingBuffer2.GetBuffer(), indexBuffer.GetBuffer(), 1, &cpyIndices);
			}, &info);
	}

	void VulkanVertexBuffer::CreateAttributeBuffers(const Mesh& mesh)
	{
		int idx = 1;
		for (auto& attr : mesh.attributes)
		{
			VkFormat format = VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT;
			size_t size = attr->GetSize();
			const void* data = attr->GetData();

			switch (attr->GetStride())
			{
			case 4:
				if (attr->isInteger)
					format = VkFormat::VK_FORMAT_R32_SINT;
				else
					format = VkFormat::VK_FORMAT_R32_SFLOAT;
				break;
			case 8:
				format = VkFormat::VK_FORMAT_R32G32_SFLOAT;
				break;
			case 12:
				format = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
				break;
			case 16:
				format = VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT;
				break;
			default:
				continue;
			}

			VkVertexInputBindingDescription bindingDesc{};
			bindingDesc.binding = idx;
			bindingDesc.stride = static_cast<uint32_t>(attr->GetStride());
			bindingDesc.inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;
			mBindingDescriptions.push_back(bindingDesc);

			VkVertexInputAttributeDescription attrDesc{};
			attrDesc.binding = idx;
			attrDesc.location = 0;
			attrDesc.format = format;
			attrDesc.offset = 0;
			mAttribDescriptions.push_back(attrDesc);

			impl::VulkanBuffer stagingBuffer{ renderer.GetDevice(), renderer.GetGPU(), renderer.GetAllocator() };
			stagingBuffer.Create(size, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_SHARING_MODE_EXCLUSIVE,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			stagingBuffer.SetData(data);

			buffers.push_back(impl::VulkanBuffer{ renderer.GetDevice(), renderer.GetGPU(), renderer.GetAllocator() });
			buffers.back().Create(size,
				VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_SHARING_MODE_EXCLUSIVE,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

			VkCommandBufferBeginInfo info{};
			info.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			cmd.Submit(renderer.GetGraphicsQueue(), [&]() {
				VkBufferCopy cpy{};
				cpy.srcOffset = 0; // Optional
				cpy.dstOffset = 0; // Optional
				cpy.size = size;
				vkCmdCopyBuffer(cmd.GetCommandBuffer(), stagingBuffer.GetBuffer(), buffers.back().GetBuffer(), 1, &cpy);
			}, &info);

			++idx;
		}
	}

	void VulkanVertexBuffer::Create(const Mesh& mesh)
	{
		Clean();

		cmd.Create();
		CreateVertexBuffer(mesh);
		CreateAttributeBuffers(mesh);
	}

	void VulkanVertexBuffer::Bind()
	{
		std::vector<VkBuffer> vertexBuffers;
		std::vector<VkDeviceSize> offsets;
		vertexBuffers.resize(buffers.size());
		offsets.resize(buffers.size());
		for (int i = 0; i < vertexBuffers.size(); ++i)
		{
			vertexBuffers[i] = buffers[i].GetBuffer();
			offsets[i] = 0;
		}
		vkCmdBindVertexBuffers(renderer.GetCommandBuffer(), 0, vertexBuffers.size(), vertexBuffers.data(), offsets.data());
		vkCmdBindIndexBuffer(renderer.GetCommandBuffer(), indexBuffer.GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
	}
}