#include "VulkanBuffer.h"
#include "VulkanContext.h"

#include <cassert>
#include <cstring>
#include <stdexcept>

namespace sh::render::vk
{
	VulkanBuffer::VulkanBuffer(const VulkanContext& context) :
		context(context),
		buffer(nullptr), bufferMem(nullptr), dataPtr(nullptr),
		bufferInfo(), memProperty(0),
		persistentMapping(false),
		size(0)
	{
	}
	VulkanBuffer::VulkanBuffer(const VulkanBuffer& other) :
		context(other.context),
		buffer(nullptr), bufferMem(nullptr), dataPtr(nullptr),
		bufferInfo(other.bufferInfo), memProperty(other.memProperty),
		persistentMapping(other.persistentMapping),
		size(other.size)
	{
		operator=(other);
	}
	VulkanBuffer::VulkanBuffer(VulkanBuffer&& other) noexcept :
		context(other.context),
		buffer(other.buffer), bufferMem(other.bufferMem), dataPtr(other.dataPtr),
		bufferInfo(other.bufferInfo), memProperty(other.memProperty),
		persistentMapping(other.persistentMapping),
		size(other.size)
	{
		other.buffer = nullptr;
		other.bufferMem = nullptr;
		other.dataPtr = nullptr;
	}
	VulkanBuffer::~VulkanBuffer()
	{
		Clean();
	}
	SH_RENDER_API auto VulkanBuffer::operator=(const VulkanBuffer& other) -> VulkanBuffer&
	{
		Create(other.size, other.bufferInfo.usage, other.bufferInfo.sharingMode, other.memProperty, other.persistentMapping);
		SetData(other.dataPtr);
		
		return *this;
	}
	SH_RENDER_API auto VulkanBuffer::operator=(VulkanBuffer&& other) noexcept -> VulkanBuffer&
	{
		buffer = other.buffer;
		bufferMem = other.bufferMem;
		dataPtr = other.dataPtr;
		bufferInfo = other.bufferInfo;
		memProperty = other.memProperty;
		persistentMapping = other.persistentMapping;
		size = other.size;

		other.buffer = nullptr;
		other.bufferMem = nullptr;
		other.dataPtr = nullptr;

		return *this;
	}
	SH_RENDER_API void VulkanBuffer::Clean()
	{
		if (buffer)
		{
			vmaDestroyBuffer(context.GetAllocator(), buffer, bufferMem);
			//vkDestroyBuffer(device, buffer, nullptr);
			buffer = nullptr;
			bufferMem = nullptr;
			dataPtr = nullptr;
		}

		/*if (bufferMem)
		{
			vkFreeMemory(device, bufferMem, nullptr);
			bufferMem = nullptr;
		}*/
	}
	SH_RENDER_API auto VulkanBuffer::Create(size_t size, VkBufferUsageFlags usageBits, VkSharingMode sharing, VkMemoryPropertyFlags memPropFlagBits, bool persistentMapping) -> VkResult
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

		bool bUseMap = (memPropFlagBits & VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO;
		allocCreateInfo.flags = 0;
		if (bUseMap)
			allocCreateInfo.flags |= VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
		if (persistentMapping)
			allocCreateInfo.flags |= VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VmaAllocationInfo allocInfo{};
		auto result = vmaCreateBuffer(context.GetAllocator(), &bufferInfo, &allocCreateInfo, &buffer, &bufferMem, &allocInfo);

		assert(result == VkResult::VK_SUCCESS);
		
		if (persistentMapping)
			dataPtr = allocInfo.pMappedData;
		//result = vkBindBufferMemory(device, buffer, bufferMem, 0);
		//assert(result == VkResult::VK_SUCCESS);
		
		return result;
	}
	SH_RENDER_API void VulkanBuffer::SetData(const void* data)
	{
		assert(buffer);
		if (buffer == nullptr)
			return;

		if (!persistentMapping)
		{
			//vkMapMemory(device, bufferMem, 0, bufferInfo.size, 0, &this->data);
			vmaMapMemory(context.GetAllocator(), bufferMem, &this->dataPtr);
			std::memcpy(this->dataPtr, data, static_cast<size_t>(bufferInfo.size));
			//vkUnmapMemory(device, bufferMem);
			vmaUnmapMemory(context.GetAllocator(), bufferMem);

		}
		else
			std::memcpy(this->dataPtr, data, static_cast<size_t>(bufferInfo.size));
	}
	SH_RENDER_API auto VulkanBuffer::GetData() const -> void*
	{
		return dataPtr;
	}
	SH_RENDER_API auto VulkanBuffer::GetBuffer() const -> VkBuffer
	{
		return buffer;
	}
	SH_RENDER_API auto VulkanBuffer::GetBufferInfo() const -> const VkBufferCreateInfo&
	{
		return bufferInfo;
	}
	SH_RENDER_API auto VulkanBuffer::GetBufferMemory() const -> VmaAllocation
	{
		return bufferMem;
	}
	SH_RENDER_API auto VulkanBuffer::GetSize() const -> size_t
	{
		return size;
	}
	auto VulkanBuffer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) -> uint32_t
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(context.GetGPU(), &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
				return i;
		}

		throw std::runtime_error("Failed to find suitable memory type!");
	}
}