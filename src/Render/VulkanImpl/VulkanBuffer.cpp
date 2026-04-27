#include "VulkanBuffer.h"
#include "VulkanContext.h"
#include "VulkanCommandBufferPool.h"
#include "VulkanCommandBuffer.h"
#include "VulkanQueueManager.h"

#include "Core/Logger.h"

#include <cassert>
#include <cstring>
#include <stdexcept>

namespace sh::render::vk
{
	VulkanBuffer::VulkanBuffer(const VulkanContext& context) :
		context(context),
		buffer(nullptr), bufferMem(nullptr), dataPtr(nullptr),
		bufferInfo(), memProperty(0),
		bPersistentMapping(false),
		size(0)
	{
	}
	VulkanBuffer::VulkanBuffer(const VulkanBuffer& other) :
		context(other.context),
		buffer(nullptr), bufferMem(nullptr), dataPtr(nullptr),
		bufferInfo(other.bufferInfo), memProperty(other.memProperty),
		bPersistentMapping(other.bPersistentMapping),
		size(other.size)
	{
		operator=(other);
	}
	VulkanBuffer::VulkanBuffer(VulkanBuffer&& other) noexcept :
		context(other.context),
		buffer(other.buffer), bufferMem(other.bufferMem), dataPtr(other.dataPtr),
		bufferInfo(other.bufferInfo), memProperty(other.memProperty),
		bPersistentMapping(other.bPersistentMapping),
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
		if (&other == this)
			return *this;

		Create(other.size, other.bufferInfo.usage, other.bufferInfo.sharingMode, other.memProperty, other.bPersistentMapping);
		std::vector<uint8_t> temp = other.GetData();
		SetData(temp.data());
		
		return *this;
	}
	SH_RENDER_API auto VulkanBuffer::operator=(VulkanBuffer&& other) noexcept -> VulkanBuffer&
	{
		if (&other == this)
			return *this;

		Clean();

		buffer = other.buffer;
		bufferMem = other.bufferMem;
		dataPtr = other.dataPtr;
		bufferInfo = other.bufferInfo;
		memProperty = other.memProperty;
		bPersistentMapping = other.bPersistentMapping;
		size = other.size;

		other.buffer = nullptr;
		other.bufferMem = nullptr;
		other.dataPtr = nullptr;

		return *this;
	}
	SH_RENDER_API auto VulkanBuffer::Resize(std::size_t size) -> bool
	{
		Clean();
		const VkResult result = Create(size, bufferInfo.usage, bufferInfo.sharingMode, memProperty, this->bPersistentMapping);
		if (result == VkResult::VK_SUCCESS)
			return true;
		return false;
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
	SH_RENDER_API auto VulkanBuffer::Create(size_t size, VkBufferUsageFlags usageBits, VkSharingMode sharing, VkMemoryPropertyFlags memPropFlagBits, bool bPersistentMapping) -> VkResult
	{
		this->size = size;
		this->bPersistentMapping = bPersistentMapping;
		this->memProperty = memPropFlagBits;

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

		const bool bUseMap = (memPropFlagBits & VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		const bool bGPUOnly = (memPropFlagBits & VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		if (bGPUOnly)
			this->bPersistentMapping = false;

		VmaAllocationCreateInfo allocCreateInfo{};
		allocCreateInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_AUTO;
		allocCreateInfo.flags = 0;
		if (bUseMap)
			allocCreateInfo.flags |= VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
		if (this->bPersistentMapping)
			allocCreateInfo.flags |= VmaAllocationCreateFlagBits::VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VmaAllocationInfo allocInfo{};
		VkResult result = vmaCreateBuffer(context.GetAllocator(), &bufferInfo, &allocCreateInfo, &buffer, &bufferMem, &allocInfo);
		if (result != VkResult::VK_SUCCESS)
		{
			assert(result == VkResult::VK_SUCCESS);
			SH_ERROR_FORMAT("Failed to create buffer!: {}", string_VkResult(result));
			return result;
		}
		
		if (this->bPersistentMapping)
			dataPtr = allocInfo.pMappedData;
		//result = vkBindBufferMemory(device, buffer, bufferMem, 0);
		//assert(result == VkResult::VK_SUCCESS);
		
		return result;
	}
	SH_RENDER_API void VulkanBuffer::SetData(const void* data)
	{
		SetData(data, 0, static_cast<std::size_t>(bufferInfo.size));
	}
	SH_RENDER_API void VulkanBuffer::SetData(const void* data, std::size_t offset, std::size_t size)
	{
		assert(buffer);
		if (buffer == nullptr)
			return;

		const bool bGPUOnly = (memProperty & VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		if (!bGPUOnly)
		{
			if (!bPersistentMapping)
			{
				vmaMapMemory(context.GetAllocator(), bufferMem, &this->dataPtr);
				std::memcpy(reinterpret_cast<uint8_t*>(dataPtr) + offset, data, size);
				vmaUnmapMemory(context.GetAllocator(), bufferMem);
				dataPtr = nullptr;

			}
			else
				std::memcpy(reinterpret_cast<uint8_t*>(dataPtr) + offset, data, size);
		}
		else
		{
			VulkanBuffer stagingBuffer{ context };
			stagingBuffer.Create(size, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
				VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			stagingBuffer.SetData(data);

			VulkanCommandBuffer& cmd = *context.GetCommandBufferPool().AllocateCommandBuffer(std::this_thread::get_id(), VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT);
			cmd.Build(
				[&]()
				{
					VkBufferCopy cpy{};
					cpy.srcOffset = 0;
					cpy.dstOffset = offset;
					cpy.size = size;
					vkCmdCopyBuffer(cmd.GetCommandBuffer(), stagingBuffer.GetBuffer(), GetBuffer(), 1, &cpy);
				}
			, true);
			VkFence fence = cmd.GetOrCreateFence();
			context.GetQueueManager().Submit(VulkanQueueManager::Role::Transfer, cmd, fence);
			vkWaitForFences(context.GetDevice(), 1, &fence, true, std::numeric_limits<uint64_t>::max());

			context.GetCommandBufferPool().DeallocateCommandBuffer(cmd);
		}
	}
	SH_RENDER_API auto VulkanBuffer::GetData() const -> std::vector<uint8_t>
	{
		if (buffer == VK_NULL_HANDLE)
			return {};

		const bool bGPUOnly = (memProperty & VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		const std::size_t size = static_cast<std::size_t>(bufferInfo.size);
		std::vector<uint8_t> result;

		if (dataPtr != nullptr)
		{
			result.resize(size);
			result.assign(reinterpret_cast<uint8_t*>(dataPtr), reinterpret_cast<uint8_t*>(dataPtr) + size);
		}
		else
		{
			if (bGPUOnly)
			{
				VulkanBuffer temp{ context };
				temp.Create(size, VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT, VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
					VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, true);

				VulkanCommandBuffer& cmd = *context.GetCommandBufferPool().AllocateCommandBuffer(std::this_thread::get_id(), VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT);
				cmd.Build(
					[&]()
					{
						VkBufferCopy cpy{};
						cpy.srcOffset = 0;
						cpy.dstOffset = 0;
						cpy.size = size;
						vkCmdCopyBuffer(cmd.GetCommandBuffer(), GetBuffer(), temp.GetBuffer(), 1, &cpy);
					}
				, true);
				VkFence fence = cmd.GetOrCreateFence();
				context.GetQueueManager().Submit(VulkanQueueManager::Role::Transfer, cmd, fence);
				vkWaitForFences(context.GetDevice(), 1, &fence, true, std::numeric_limits<uint64_t>::max());

				context.GetCommandBufferPool().DeallocateCommandBuffer(cmd);
				return temp.GetData();
			}

			result.resize(size);
			void* tempPtr = nullptr;
			vmaMapMemory(context.GetAllocator(), bufferMem, &tempPtr);
			result.assign(reinterpret_cast<uint8_t*>(tempPtr), reinterpret_cast<uint8_t*>(tempPtr) + size);
			vmaUnmapMemory(context.GetAllocator(), bufferMem);
		}
		return result;
	}

	//auto VulkanBuffer::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) -> uint32_t
	//{
	//	VkPhysicalDeviceMemoryProperties memProperties;
	//	vkGetPhysicalDeviceMemoryProperties(context.GetGPU(), &memProperties);

	//	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
	//		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
	//			return i;
	//	}

	//	throw std::runtime_error("Failed to find suitable memory type!");
	//}
}//namespace