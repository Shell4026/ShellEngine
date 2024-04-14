#pragma once

#include "Export.h"

#include <vulkan/vulkan.h>
#include <functional>

namespace sh::render::impl {
	class SH_RENDER_API VulkanCommandBuffer
	{
	private:
		VkCommandBuffer buffer;

		VkDevice device;
		VkCommandPool cmdPool;
	private:
		auto Begin(VkCommandBufferBeginInfo* info = nullptr) -> VkResult;
		auto End() -> VkResult;
	public:
		VulkanCommandBuffer(VkDevice device, VkCommandPool pool);
		~VulkanCommandBuffer();

		auto Submit(VkQueue queue, const std::function<void()>& commands, const VkSubmitInfo* info = nullptr, VkFence fence = nullptr) -> VkResult;

		auto Create(const VkCommandBufferAllocateInfo* info = nullptr) -> VkResult;
		auto Reset() -> VkResult;

		auto GetCommandBuffer() const-> VkCommandBuffer;
	};
}