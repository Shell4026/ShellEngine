#include "VulkanImpl/VulkanBuffer.h"

#include <cassert>
#include <cstring>
#include <stdexcept>

namespace sh::render::impl
{
	VulkanBuffer::VulkanBuffer(VkDevice device, VkPhysicalDevice gpu, VmaAllocator allocator) :
		device(device), gpu(gpu), allocator(allocator),
		buffer(nullptr), bufferMem(nullptr), data(nullptr),
		bufferInfo(), memProperty(0),
		persistentMapping(false),
		size(0)
	{
	}
	VulkanBuffer::VulkanBuffer(const VulkanBuffer& other) :
		device(other.device), gpu(other.gpu), allocator(other.allocator),
		buffer(nullptr), bufferMem(nullptr), data(nullptr),
		bufferInfo(other.bufferInfo), memProperty(other.memProperty),
		persistentMapping(other.persistentMapping),
		size(other.size)
	{
		operator=(other);
	}
	VulkanBuffer::VulkanBuffer(VulkanBuffer&& other) noexcept :
		device(other.device), gpu(other.gpu), allocator(other.allocator),
		buffer(other.buffer), bufferMem(other.bufferMem), data(other.data),
		bufferInfo(other.bufferInfo), memProperty(other.memProperty),
		persistentMapping(other.persistentMapping),
		size(other.size)
	{
		other.device = nullptr;
		other.gpu = nullptr;
		other.allocator = nullptr;

		other.buffer = nullptr;
		other.bufferMem = nullptr;
		other.data = nullptr;
	}
	VulkanBuffer::~VulkanBuffer()
	{
		Clean();
	}
	auto VulkanBuffer::operator=(const VulkanBuffer& other)->VulkanBuffer&
	{
		Create(size, bufferInfo.usage, bufferInfo.sharingMode, memProperty, persistentMapping);
		SetData(other.data);
		
		return *this;
	}
	auto VulkanBuffer::operator=(VulkanBuffer&& other) noexcept -> VulkanBuffer&
	{
		device = other.device;
		gpu = other.gpu;
		allocator = other.allocator;

		buffer = other.buffer;
		bufferMem = other.bufferMem;
		data = other.data;
		bufferInfo = other.bufferInfo;
		memProperty = other.memProperty;
		persistentMapping = other.persistentMapping;
		size = other.size;

		other.device = nullptr;
		other.gpu = nullptr;
		other.allocator = nullptr;

		other.buffer = nullptr;
		other.bufferMem = nullptr;
		other.data = nullptr;

		return *this;
	}

	void VulkanBuffer::Clean()
	{
		if (buffer)
		{
			vmaDestroyBuffer(allocator, buffer, bufferMem);
			//vkDestroyBuffer(device, buffer, nullptr);
			buffer = nullptr;
			bufferMem = nullptr;
		}

		/*if (bufferMem)
		{
			vkFreeMemory(device, bufferMem, nullptr);
			bufferMem = nullptr;
		}*/
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
		this->size = size;
		this->persistentMapping = persistentMapping;

		Clean();
		bufferInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usageBits;
		bufferInfo.sharingMode = sharing;

		/*VkResult result = vkCreateBuffer(device, &bufferInfo, nullptr, &buffer);
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

		result = vkAllocateMemory(device, &allocInfo, nullptr, &bufferMem);*/

		bool bUseMap = (memPropFlagBits & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO;
		if (bUseMap)
			allocCreateInfo.flags = VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
		if (persistentMapping)
			allocCreateInfo.flags |= VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VmaAllocationInfo allocInfo{};
		auto result = vmaCreateBuffer(allocator, &bufferInfo, &allocCreateInfo, &buffer, &bufferMem, &allocInfo);

		assert(result == VkResult::VK_SUCCESS);
		
		if (persistentMapping)
			data = allocInfo.pMappedData;
		//result = vkBindBufferMemory(device, buffer, bufferMem, 0);
		//assert(result == VkResult::VK_SUCCESS);
		
		return result;
	}

	void VulkanBuffer::SetData(const void* data)
	{
		assert(buffer);
		if (buffer == nullptr)
			return;

		if (!persistentMapping)
		{
			//vkMapMemory(device, bufferMem, 0, bufferInfo.size, 0, &this->data);
			vmaMapMemory(allocator, bufferMem, &this->data);
			std::memcpy(this->data, data, static_cast<size_t>(bufferInfo.size));
			//vkUnmapMemory(device, bufferMem);
			vmaUnmapMemory(allocator, bufferMem);

		}
		else
			std::memcpy(this->data, data, static_cast<size_t>(bufferInfo.size));
	}

	auto VulkanBuffer::GetBuffer() const -> VkBuffer
	{
		return buffer;
	}
	auto VulkanBuffer::GetBufferInfo() const -> const VkBufferCreateInfo&
	{
		return bufferInfo;
	}
	auto VulkanBuffer::GetBufferMemory() const -> VmaAllocation
	{
		return bufferMem;
	}
	auto VulkanBuffer::GetSize() const -> size_t
	{
		return size;
	}
}