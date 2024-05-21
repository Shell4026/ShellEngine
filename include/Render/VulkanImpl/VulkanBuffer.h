#pragma once

#include "VulkanConfig.h"

#include "Render/Export.h"

#include "Core/NonCopyable.h"

#include "../vma-src/include/vk_mem_alloc.h"

namespace sh::render
{
	namespace impl
	{
		class VulkanBuffer : public sh::core::INonCopyable
		{
		private:
			VkDevice device;
			VkPhysicalDevice gpu;
			VmaAllocator allocator;

			VkBuffer buffer;
			VmaAllocation bufferMem;
			VkBufferCreateInfo bufferInfo;

			void* data;
			bool persistentMapping;
		private:
			auto FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) -> uint32_t;
		public:
			SH_RENDER_API VulkanBuffer(VkDevice device, VkPhysicalDevice gpu, VmaAllocator allocator);
			SH_RENDER_API VulkanBuffer(VulkanBuffer&& other) noexcept;
			SH_RENDER_API ~VulkanBuffer();

			SH_RENDER_API auto Create(size_t size, VkBufferUsageFlags usageBits, VkSharingMode sharing, VkMemoryPropertyFlags memPropFlagBits, bool persistentMapping = false) -> VkResult;
			SH_RENDER_API void Clean();
			SH_RENDER_API void SetData(const void* data);
			SH_RENDER_API auto GetBuffer() const -> VkBuffer;
			SH_RENDER_API auto GetBufferMemory() const -> VmaAllocation;
		};
	}
}