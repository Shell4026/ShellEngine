#pragma once

#include "VulkanConfig.h"
#include "IBuffer.h"

#include "Render/Export.h"

namespace sh::render
{
	namespace impl
	{
		class VulkanBuffer : public IBuffer
		{
		private:
			VkDevice device;
			VkPhysicalDevice gpu;
			VmaAllocator allocator;

			VkBuffer buffer;
			VmaAllocation bufferMem;
			VkBufferCreateInfo bufferInfo;
			VkMemoryPropertyFlags memProperty;

			size_t size;

			void* data;
			bool persistentMapping; // 즉시 맵핑
		private:
			auto FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) -> uint32_t;
		public:
			SH_RENDER_API VulkanBuffer(VkDevice device, VkPhysicalDevice gpu, VmaAllocator allocator);
			SH_RENDER_API VulkanBuffer(const VulkanBuffer& other);
			SH_RENDER_API VulkanBuffer(VulkanBuffer&& other) noexcept;
			SH_RENDER_API ~VulkanBuffer();

			SH_RENDER_API auto operator=(const VulkanBuffer& other) -> VulkanBuffer&;
			SH_RENDER_API auto operator=(VulkanBuffer&& other) noexcept -> VulkanBuffer&;

			SH_RENDER_API auto Create(size_t size, VkBufferUsageFlags usageBits, VkSharingMode sharing, VkMemoryPropertyFlags memPropFlagBits, bool persistentMapping = false) -> VkResult;
			SH_RENDER_API void Clean();
			SH_RENDER_API void SetData(const void* data);
			SH_RENDER_API auto GetData() const -> void* override;
			SH_RENDER_API auto GetBuffer() const -> VkBuffer;
			SH_RENDER_API auto GetBufferInfo() const -> const VkBufferCreateInfo&;
			SH_RENDER_API auto GetBufferMemory() const -> VmaAllocation;
			SH_RENDER_API auto GetSize() const -> size_t;
		};
	}
}