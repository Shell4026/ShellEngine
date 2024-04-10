#pragma once

#include "Export.h"

#include <vulkan/vulkan.h>

namespace sh::render {
	class SH_RENDER_API CommandBuffer
	{
	private:
		VkCommandBuffer buffer;
	public:
		CommandBuffer(const VkDevice device, const VkCommandPool pool, const VkCommandBufferAllocateInfo* info = nullptr);

		auto submit(VkQueue queue, const VkSubmitInfo* info = nullptr, const VkFence fence = VK_NULL_HANDLE) -> VkResult;
	};
}