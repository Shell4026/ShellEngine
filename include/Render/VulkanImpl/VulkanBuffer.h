#pragma once
#include "VulkanConfig.h"

#include "Render/IBuffer.h"
#include "Render/Export.h"

namespace sh::render::vk
{
	class VulkanContext;
	class VulkanBuffer : public IBuffer
	{
	public:
		SH_RENDER_API VulkanBuffer(const VulkanContext& context);
		SH_RENDER_API VulkanBuffer(const VulkanBuffer& other);
		SH_RENDER_API VulkanBuffer(VulkanBuffer&& other) noexcept;
		SH_RENDER_API ~VulkanBuffer();

		SH_RENDER_API auto operator=(const VulkanBuffer& other)->VulkanBuffer&;
		SH_RENDER_API auto operator=(VulkanBuffer&& other) noexcept -> VulkanBuffer&;

		SH_RENDER_API auto Resize(std::size_t size) -> bool override;

		SH_RENDER_API auto Create(size_t size, VkBufferUsageFlags usageBits, VkSharingMode sharing, VkMemoryPropertyFlags memPropFlagBits, bool persistentMapping = false) -> VkResult;
		SH_RENDER_API void Clean();
		SH_RENDER_API void SetData(const void* data);

		SH_RENDER_API auto GetData() const -> void* override { return dataPtr; }
		SH_RENDER_API auto GetBuffer() const -> VkBuffer { return buffer; }
		SH_RENDER_API auto GetBufferInfo() const -> const VkBufferCreateInfo& { return bufferInfo; }
		SH_RENDER_API auto GetBufferMemory() const -> VmaAllocation { return bufferMem; }
		SH_RENDER_API auto GetSize() const -> size_t { return size; }
	private:
		auto FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) -> uint32_t;
	private:
		const VulkanContext& context;

		VkBuffer buffer;
		VmaAllocation bufferMem;
		VkBufferCreateInfo bufferInfo;
		VkMemoryPropertyFlags memProperty;

		size_t size;

		void* dataPtr;
		bool persistentMapping; // 즉시 맵핑
	};
}//namespace