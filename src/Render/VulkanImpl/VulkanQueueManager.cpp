#include "VulkanQueueManager.h"
#include "VulkanCommandBuffer.h"

namespace sh::render::vk
{
	void VulkanQueueManager::GetQueueFamilyProperties()
	{
		uint32_t count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, nullptr);
		queueFamilyProps.resize(count);
		vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, queueFamilyProps.data());
	}
	auto VulkanQueueManager::SelectQueueFamily(VkQueueFlagBits queueType) -> std::optional<int>
	{
		int idx = 0;
		for (auto& prop : queueFamilyProps)
		{
			if ((prop.queueFlags & queueType) == queueType)
				return idx;
			++idx;
		}
		return {};
	}
	auto VulkanQueueManager::GetSurfaceQueueFamily(VkSurfaceKHR surface) -> std::optional<int>
	{
		assert(surface != nullptr);
		assert(gpu != nullptr);
		int idx = 0;
		for (auto& prop : queueFamilyProps)
		{
			VkBool32 support = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(gpu, idx, surface, &support);
			if (support)
				return idx;
			++idx;
		}
		return {};
	}
	SH_RENDER_API auto VulkanQueueManager::GetGraphicsQueueFamilyIdx() const -> uint8_t
	{
		return graphicsQueueFamilyIdx;
	}
	SH_RENDER_API auto VulkanQueueManager::GetTransferQueueFamilyIdx() const -> uint8_t
	{
		return transferQueueFamilyIdx;
	}
	SH_RENDER_API auto VulkanQueueManager::GetSurfaceQueueFamilyIdx() const -> uint8_t
	{
		return surfaceQueueFamilyIdx;
	}

	VulkanQueueManager::VulkanQueueManager(VkPhysicalDevice gpu) :
		gpu(gpu)
	{
		assert(gpu != nullptr);

		GetQueueFamilyProperties();
	}
	VulkanQueueManager::VulkanQueueManager(VulkanQueueManager&& other) noexcept :
		device(other.device), gpu(other.gpu),
		graphicsQueue(other.graphicsQueue),
		surfaceQueue(other.surfaceQueue),
		transferQueue(other.transferQueue)
	{
		other.device = nullptr;
		other.gpu = nullptr;

		other.graphicsQueue = nullptr;
		other.surfaceQueue = nullptr;
		other.transferQueue = nullptr;

		if (auto idx = SelectQueueFamily(VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT); !idx.has_value())
			throw std::runtime_error("VulkanQueueManager - VK_QUEUE_GRAPHICS_BIT not found!");
		else
			graphicsQueueFamilyIdx = *idx;

		if (auto idx = SelectQueueFamily(VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT); !idx.has_value())
			throw std::runtime_error("VulkanQueueManager - VK_QUEUE_TRANSFER_BIT not found!");
		else
			transferQueueFamilyIdx = *idx;
	}
	VulkanQueueManager::~VulkanQueueManager()
    {
    }
	SH_RENDER_API void VulkanQueueManager::SetDevice(VkDevice device)
	{
		this->device = device;
	}
	SH_RENDER_API void VulkanQueueManager::CreateGraphicsQueue()
	{
		assert(device != nullptr);
		vkGetDeviceQueue(device, graphicsQueueFamilyIdx, 0, &graphicsQueue);
		assert(graphicsQueue);
	}
	SH_RENDER_API void VulkanQueueManager::CreateTransferQueue()
	{
		assert(device != nullptr);
		vkGetDeviceQueue(device, graphicsQueueFamilyIdx, 0, &transferQueue);
		assert(graphicsQueue);
	}
	SH_RENDER_API void VulkanQueueManager::CreateSurfaceQueue(VkSurfaceKHR surface)
	{
		assert(device != nullptr);

		if (auto idx = GetSurfaceQueueFamily(surface); !idx.has_value())
			throw std::runtime_error("VulkanQueueManager - surface queue family not found!");
		else
			surfaceQueueFamilyIdx = *idx;

		vkGetDeviceQueue(device, graphicsQueueFamilyIdx, 0, &surfaceQueue);
		assert(graphicsQueue);
	}
	SH_RENDER_API auto VulkanQueueManager::GetGraphicsQueue() const -> VkQueue
	{
		return graphicsQueue;
	}
	SH_RENDER_API auto VulkanQueueManager::GetTransferQueue() const -> VkQueue
	{
		return transferQueue;
	}
	SH_RENDER_API auto VulkanQueueManager::GetSurfaceQueue() const -> VkQueue
	{
		return surfaceQueue;
	}
	SH_RENDER_API void VulkanQueueManager::SubmitCommand(const VulkanCommandBuffer& cmd, VkFence fence)
	{
		spinLock.Lock();

		const VkPipelineStageFlags waitStage = cmd.GetWaitStage();
		const VkCommandBuffer commandBuffer = cmd.GetCommandBuffer();
		auto& waitSemaphores = cmd.GetWaitSemaphore();
		auto& signalSemaphores = cmd.GetSignalSemaphore();

		VkSubmitInfo sinfo{};
		sinfo.sType = VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO;
		sinfo.pNext = nullptr;
		sinfo.pWaitDstStageMask = &waitStage;
		sinfo.commandBufferCount = 1;
		sinfo.pCommandBuffers = &commandBuffer;
		sinfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
		sinfo.pWaitSemaphores = waitSemaphores.data();
		sinfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
		sinfo.pSignalSemaphores = signalSemaphores.data();

		VkQueue queue = nullptr;
		auto queueType = cmd.GetQueueType();
		if (queueType == VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
			queue = graphicsQueue;
		else if (queueType == VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT)
			queue = transferQueue;
		assert(queue != nullptr);

		auto result = vkQueueSubmit(queue, 1, &sinfo, fence);
		assert(result == VkResult::VK_SUCCESS);
		spinLock.UnLock();

		vkQueueWaitIdle(queue);
	}
}//namespace