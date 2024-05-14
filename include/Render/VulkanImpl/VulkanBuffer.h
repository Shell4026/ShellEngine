#pragma once

#include "VulkanConfig.h"

#include "Render/Export.h"

#include "Core/NonCopyable.h"

namespace sh::render
{
	namespace impl
	{
		class VulkanBuffer : public sh::core::INonCopyable
		{
		private:
			VkDevice device;
			VkPhysicalDevice gpu;
			VkBuffer buffer;
			VkDeviceMemory bufferMem;
			VkBufferCreateInfo bufferInfo;
		public:
			SH_RENDER_API VulkanBuffer(VkDevice device, VkPhysicalDevice gpu);
			SH_RENDER_API VulkanBuffer(VulkanBuffer&& other) noexcept;
			SH_RENDER_API ~VulkanBuffer();

			SH_RENDER_API auto Create(size_t size, int usageBits, VkSharingMode sharing, int memPropFlagBits) -> VkResult;
			SH_RENDER_API void Clean();
			SH_RENDER_API void SetData(const void* data);
			SH_RENDER_API auto GetBuffer() const -> VkBuffer;
			SH_RENDER_API auto GetBufferMemory() const -> VkDeviceMemory;
		};
	}
}