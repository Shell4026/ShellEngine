#include "VulkanImpl/VulkanBuffer.h"

#include <cassert>
#include <cstring>
#include <stdexcept>

namespace sh::render::impl
{
	VulkanBuffer::VulkanBuffer(VkDevice device, VkPhysicalDevice gpu) :
		device(device), gpu(gpu), 
		buffer(nullptr), bufferMem(nullptr), data(nullptr),
		bufferInfo(),
		persistentMapping(false)
	{

	}

	VulkanBuffer::VulkanBuffer(VulkanBuffer&& other) noexcept :
		device(other.device), gpu(other.gpu),
		buffer(other.buffer), bufferMem(other.bufferMem), data(other.data),
		bufferInfo(other.bufferInfo),
		persistentMapping(other.persistentMapping)
	{
		other.device = nullptr;
		other.gpu = nullptr;
		other.buffer = nullptr;
		other.bufferMem = nullptr;
		other.data = nullptr;
	}

	VulkanBuffer::~VulkanBuffer()
	{
		Clean();
	}

	void VulkanBuffer::Clean()
	{
		if (buffer)
		{
			vkDestroyBuffer(device, buffer, nullptr);
			buffer = nullptr;
		}

		if (bufferMem)
		{
			vkFreeMemory(device, bufferMem, nullptr);
			bufferMem = nullptr;
		}
	}

	auto VulkanBuffer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) -> uint32_t
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(gpu, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				return i;
		}

		throw std::runtime_error("Failed to find suitable memory type!");
	}

	auto VulkanBuffer::Create(size_t size, VkBufferUsageFlags usageBits, VkSharingMode sharing, VkMemoryPropertyFlags memPropFlagBits, bool persistentMapping) -> VkResult
	{
		this->persistentMapping = persistentMapping;

		Clean();
		bufferInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usageBits;
		bufferInfo.sharingMode = sharing;

		VkResult result = vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);
		assert(result == VK_SUCCESS);
		if (result != VK_SUCCESS)
			return result;

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		uint32_t idx = FindMemoryType(memRequirements.memoryTypeBits, memPropFlagBits);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = idx;

		result = vkAllocateMemory(device, &allocInfo, nullptr, &bufferMem);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VK_SUCCESS)
			return result;

		result = vkBindBufferMemory(device, buffer, bufferMem, 0);
		assert(result == VkResult::VK_SUCCESS);
		if(persistentMapping)
			vkMapMemory(device, bufferMem, 0, bufferInfo.size, 0, &data);

		return result;
	}

	void VulkanBuffer::SetData(const void* data)
	{
		assert(buffer);
		if (buffer == nullptr)
			return;

		if (!persistentMapping)
		{
			vkMapMemory(device, bufferMem, 0, bufferInfo.size, 0, &this->data);
			std::memcpy(this->data, data, static_cast<size_t>(bufferInfo.size));
			vkUnmapMemory(device, bufferMem);
		}
		else
			std::memcpy(this->data, data, static_cast<size_t>(bufferInfo.size));
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