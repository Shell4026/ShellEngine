#include "VulkanCommandBuffer.h"

#include <cassert>

namespace sh::render {
	CommandBuffer::CommandBuffer(const VkDevice device, const VkCommandPool pool, const VkCommandBufferAllocateInfo* info) :
		buffer(nullptr)
	{
		VkResult result;
		if (info)
		{
			result = vkAllocateCommandBuffers(device, info, &buffer);
			assert(result == VkResult::VK_SUCCESS);
			return;
		}

		VkCommandBufferAllocateInfo cmdInfo = {};
		cmdInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdInfo.pNext = nullptr;
		cmdInfo.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdInfo.commandPool = pool;
		cmdInfo.commandBufferCount = 1;

		result = vkAllocateCommandBuffers(device, info, &buffer);
		assert(result == VkResult::VK_SUCCESS);
	}

	auto CommandBuffer::submit(VkQueue queue, const VkSubmitInfo* info, const VkFence fence) -> VkResult
	{
		VkResult result;
		if (info)
		{
			vkQueueSubmit(queue, 1, info, fence);
			result = vkQueueWaitIdle(queue);
			return result;
		}

		VkSubmitInfo sinfo{};
		sinfo.sType = VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO;
		sinfo.pNext = nullptr;
		sinfo.waitSemaphoreCount = 0;
		sinfo.pWaitSemaphores = nullptr;
		sinfo.pWaitDstStageMask = nullptr;
		sinfo.commandBufferCount = 1;
		sinfo.pCommandBuffers = &buffer;
		sinfo.signalSemaphoreCount = 0;
		sinfo.pSignalSemaphores = nullptr;
		
		result = vkQueueSubmit(queue, 1, &sinfo, fence);
		assert(result == VkResult::VK_SUCCESS);
		result = vkQueueWaitIdle(queue);
		return result;
	}
}