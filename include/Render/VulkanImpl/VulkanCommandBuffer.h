#pragma once

#include "Export.h"

#include <vulkan/vulkan.h>
#include <functional>

namespace sh::render {
	class SH_RENDER_API CommandBuffer
	{
	private:
		VkCommandBuffer buffer;
	private:
		auto Begin(VkCommandBufferBeginInfo* info = nullptr) -> VkResult;
		auto End() -> VkResult;
	public:
		CommandBuffer(const VkDevice device, const VkCommandPool pool, const VkCommandBufferAllocateInfo* info = nullptr);
		~CommandBuffer();

		auto Submit(VkQueue queue, std::function<void()>& commands, const VkSubmitInfo* info = nullptr, const VkFence fence = VK_NULL_HANDLE) -> VkResult;

		auto Reset() -> VkResult;
	};
}