#include "VulkanCommandBuffer.h"

#include <cassert>

namespace sh::render::impl {
	VulkanCommandBuffer::VulkanCommandBuffer(VkDevice device, VkCommandPool pool) :
		buffer(nullptr), device(device), cmdPool(pool), waitStage(0)
	{

	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		Clean();
	}

	auto VulkanCommandBuffer::Create(const VkCommandBufferAllocateInfo* info)->VkResult
	{
		VkResult result;
		if (info)
		{
			result = vkAllocateCommandBuffers(device, info, &buffer);
			assert(result == VkResult::VK_SUCCESS);
			return result;
		}

		VkCommandBufferAllocateInfo cmdInfo = {};
		cmdInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdInfo.pNext = nullptr;
		cmdInfo.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdInfo.commandPool = cmdPool;
		cmdInfo.commandBufferCount = 1;

		result = vkAllocateCommandBuffers(device, &cmdInfo, &buffer);
		assert(result == VkResult::VK_SUCCESS);

		return result;
	}

	auto VulkanCommandBuffer::Begin(VkCommandBufferBeginInfo* info) -> VkResult
	{
		assert(buffer);

		VkResult result;
		if (!buffer)
			return VkResult::VK_ERROR_UNKNOWN;

		if (info)
		{
			result = vkBeginCommandBuffer(buffer, info);
			assert(result == VkResult::VK_SUCCESS);
			return result;
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
		beginInfo.pInheritanceInfo = nullptr; //inheritInfo

		result = vkBeginCommandBuffer(buffer, &beginInfo);
		assert(result == VkResult::VK_SUCCESS);
		return result;
	}

	auto VulkanCommandBuffer::End() -> VkResult
	{
		assert(buffer);

		VkResult result = vkEndCommandBuffer(buffer);
		assert(result == VkResult::VK_SUCCESS);

		return result;
	}

	void VulkanCommandBuffer::SetWaitStage(const std::initializer_list<VkPipelineStageFlagBits> s)
	{
		for (auto i : s)
			waitStage |= i;
	}

	void VulkanCommandBuffer::SetWaitSemaphore(const std::initializer_list<VkSemaphore> s)
	{
		waitSemaphores.resize(s.size());
		int idx = 0;
		for (auto i : s)
			waitSemaphores[idx++] = i;
	}

	void VulkanCommandBuffer::SetSignalSemaphore(const std::initializer_list<VkSemaphore> s)
	{
		signalSemaphores.resize(s.size());
		int idx = 0;
		for (auto i : s)
			signalSemaphores[idx++] = i;
	}

	auto VulkanCommandBuffer::Submit(VkQueue queue, const std::function<void()>& commands, VkCommandBufferBeginInfo* beginInfo, VkFence fence) -> VkResult
	{
		VkResult result;
		if (result = Begin(beginInfo); result != VkResult::VK_SUCCESS) return result;
		commands();
		if (result = End(); result != VkResult::VK_SUCCESS) return result;

		VkSubmitInfo sinfo{};
		sinfo.sType = VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO;
		sinfo.pNext = nullptr;
		sinfo.pWaitDstStageMask = &waitStage;
		sinfo.commandBufferCount = 1;
		sinfo.pCommandBuffers = &buffer;
		sinfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
		sinfo.pWaitSemaphores = waitSemaphores.data();
		sinfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
		sinfo.pSignalSemaphores = signalSemaphores.data();
		
		result = vkQueueSubmit(queue, 1, &sinfo, fence);
		assert(result == VkResult::VK_SUCCESS);
		result = vkQueueWaitIdle(queue);
		return result;
	}

	auto VulkanCommandBuffer::Reset() -> VkResult
	{
		waitSemaphores.clear();
		signalSemaphores.clear();
		if (buffer)
		{
			VkResult result = vkResetCommandBuffer(buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
			assert(result == VkResult::VK_SUCCESS);

			return result;
		}

		return VkResult::VK_ERROR_UNKNOWN;
	}

	void VulkanCommandBuffer::Clean()
	{
		waitSemaphores.clear();
		signalSemaphores.clear();
		if (buffer)
		{
			vkFreeCommandBuffers(device, cmdPool, 1, &buffer);
			buffer = nullptr;
		}
	}

	auto VulkanCommandBuffer::GetCommandBuffer() const -> VkCommandBuffer
	{
		return buffer;
	}
}