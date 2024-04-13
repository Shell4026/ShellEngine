#include "VulkanImpl/VulkanCommandBuffer.h"

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

	CommandBuffer::~CommandBuffer()
	{
		Reset();
	}

	auto CommandBuffer::Begin(VkCommandBufferBeginInfo* info) -> VkResult
	{
		assert(buffer);

		VkResult result;
		if (!buffer)
			return VkResult::VK_ERROR_UNKNOWN;

		if (info)
		{
			result = vkBeginCommandBuffer(buffer, info);
			assert(result == VkResult::VK_SUCCESS);
		}

		VkCommandBufferInheritanceInfo inheritInfo{};
		inheritInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritInfo.pNext = nullptr;
		inheritInfo.renderPass = VK_NULL_HANDLE;
		inheritInfo.subpass = 0;
		inheritInfo.framebuffer = VK_NULL_HANDLE;
		inheritInfo.occlusionQueryEnable = VK_FALSE;
		inheritInfo.queryFlags = 0;
		inheritInfo.pipelineStatistics = 0;

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = nullptr;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = &inheritInfo;

		result = vkBeginCommandBuffer(buffer, info);
		assert(result == VkResult::VK_SUCCESS);
		return result;
	}

	auto CommandBuffer::End() -> VkResult
	{
		assert(buffer);

		VkResult result = vkEndCommandBuffer(buffer);
		assert(result == VkResult::VK_SUCCESS);

		return result;
	}

	auto CommandBuffer::Submit(VkQueue queue, std::function<void()>& commands, const VkSubmitInfo* info, const VkFence fence) -> VkResult
	{
		VkResult result;
		if (result = Begin(); result != VkResult::VK_SUCCESS) return result;
		commands();
		if (result = End(); result != VkResult::VK_SUCCESS) return result;

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

	auto CommandBuffer::Reset() -> VkResult
	{
		assert(buffer);
		if (buffer)
		{
			VkResult result = vkResetCommandBuffer(buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
			assert(result == VkResult::VK_SUCCESS);
			return result;
		}

		return VkResult::VK_ERROR_UNKNOWN;
	}
}