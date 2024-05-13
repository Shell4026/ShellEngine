#include "VulkanImpl/VulkanBuffer.h"

#include <cassert>
#include <cstring>

namespace sh::render::impl
{
	VulkanBuffer::VulkanBuffer(VkDevice device, VkPhysicalDevice gpu) :
		device(device), gpu(gpu), 
		buffer(nullptr), bufferMem(nullptr), bufferInfo()
	{

	}

	VulkanBuffer::~VulkanBuffer()
	{
		Clean();
	}

	void VulkanBuffer::Clean()
	{
		if (buffer != nullptr)
		{
			vkDestroyBuffer(device, buffer, nullptr);
			buffer = nullptr;
			vkFreeMemory(device, bufferMem, nullptr);
			bufferMem = nullptr;
		}
	}

	auto VulkanBuffer::Create(size_t size, VkBufferUsageFlagBits usage, VkSharingMode sharing, int memPropFlagBits) -> VkResult
	{
		bufferInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = sharing;

		VkResult result = vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);
		assert(result == VK_SUCCESS);
		if (result != VK_SUCCESS)
			return result;

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(gpu, &memProperties);

		uint32_t idx = 0;
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((memRequirements.memoryTypeBits & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & memPropFlagBits) == memPropFlagBits) {
				idx = i;
				break;
			}
		}

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = idx;

		result = vkAllocateMemory(device, &allocInfo, nullptr, &bufferMem);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VK_SUCCESS)
			return result;

		vkBindBufferMemory(device, buffer, bufferMem, 0);
	}

	void VulkanBuffer::SetData(void* data)
	{
		assert(buffer);
		if (buffer == nullptr)
			return;

		void* tmp;
		vkMapMemory(device, bufferMem, 0, bufferInfo.size, 0, &tmp);
		std::memcpy(tmp, data, static_cast<size_t>(bufferInfo.size));
		vkUnmapMemory(device, bufferMem);
	}

	auto VulkanBuffer::GetBuffer() const -> VkBuffer
	{
		return buffer;
	}

	auto VulkanBuffer::GetBufferMemory() const -> VkDeviceMemory
	{
		return bufferMem;
	}
}