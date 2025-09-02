#include "VulkanQueueManager.h"
#include "VulkanCommandBuffer.h"
#include "VulkanContext.h"

#include "Core/Logger.h"

#include <limits>
namespace sh::render::vk
{
	SH_RENDER_API auto VulkanQueueManager::GetGraphicsQueueFamily() const -> QueueFamily
	{
		return graphicsQueueFamily;
	}
	SH_RENDER_API auto VulkanQueueManager::GetTransferQueueFamily() const -> QueueFamily
	{
		return transferQueueFamily;
	}
	SH_RENDER_API auto VulkanQueueManager::GetSurfaceQueueFamily() const -> QueueFamily
	{
		return surfaceQueueFamily;
	}

	VulkanQueueManager::VulkanQueueManager(const VulkanContext& ctx) :
		ctx(ctx)
	{
		GetQueueFamilyProperties();
	}
	VulkanQueueManager::VulkanQueueManager(VulkanQueueManager&& other) noexcept:
		ctx(other.ctx),
		graphicsQueueFamily(other.graphicsQueueFamily), surfaceQueueFamily(other.surfaceQueueFamily), transferQueueFamily(other.transferQueueFamily),
		graphicsQueue(other.graphicsQueue), surfaceQueue(other.surfaceQueue), transferQueue(other.transferQueue),
		queueFamilyProps(std::move(other.queueFamilyProps))
	{
		other.graphicsQueue = nullptr;
		other.surfaceQueue = nullptr;
		other.transferQueue = nullptr;
	}
	VulkanQueueManager::~VulkanQueueManager()
    {
    }
	SH_RENDER_API void VulkanQueueManager::QueryQueueFamily(VkSurfaceKHR surface)
	{
		if (auto family = SelectQueueFamily(VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT); !family.has_value())
			throw std::runtime_error("VulkanQueueManager - VK_QUEUE_GRAPHICS_BIT not found!");
		else
			graphicsQueueFamily = *family;

		if (auto family = SelectQueueFamily(VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT); !family.has_value())
			throw std::runtime_error("VulkanQueueManager - VK_QUEUE_TRANSFER_BIT not found!");
		else
			transferQueueFamily = *family;

		if (auto family = GetSurfaceQueueFamily(surface); !family.has_value())
			throw std::runtime_error("VulkanQueueManager - surface queue family not found!");
		else
			surfaceQueueFamily = *family;
	}
	SH_RENDER_API void VulkanQueueManager::CreateGraphicsQueue()
	{
		vkGetDeviceQueue(ctx.GetDevice(), graphicsQueueFamily.idx, 0, &graphicsQueue);
	}
	SH_RENDER_API void VulkanQueueManager::CreateTransferQueue()
	{
		if (graphicsQueueFamily.idx == transferQueueFamily.idx)
		{
			if (transferQueueFamily.queueCount >= 2)
			{
				vkGetDeviceQueue(ctx.GetDevice(), transferQueueFamily.idx, 1, &transferQueue);
				return;
			}
			vkGetDeviceQueue(ctx.GetDevice(), transferQueueFamily.idx, 0, &transferQueue); // 그래픽스 큐와 같음
		}
		vkGetDeviceQueue(ctx.GetDevice(), transferQueueFamily.idx, 0, &transferQueue); // 완전 다른 패밀리 그룹임
	}
	SH_RENDER_API void VulkanQueueManager::CreateSurfaceQueue(VkSurfaceKHR surface)
	{
		if (graphicsQueueFamily.idx == surfaceQueueFamily.idx)
		{
			if (surfaceQueueFamily.queueCount >= 3)
			{
				vkGetDeviceQueue(ctx.GetDevice(), surfaceQueueFamily.idx, 2, &surfaceQueue);
				return;
			}
			vkGetDeviceQueue(ctx.GetDevice(), surfaceQueueFamily.idx, 0, &surfaceQueue); // 그래픽스 큐와 같음
		}
		vkGetDeviceQueue(ctx.GetDevice(), surfaceQueueFamily.idx, 0, &surfaceQueue); // 완전 다른 패밀리 그룹임
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
	SH_RENDER_API void VulkanQueueManager::SubmitCommand(VkQueue queue, const VulkanCommandBuffer& cmd, VkFence fence)
	{
		const VkPipelineStageFlags waitStage = cmd.GetWaitStage();
		const VkCommandBuffer commandBuffer = cmd.GetCommandBuffer();
		auto& waitSemaphoreInfos = cmd.GetWaitSemaphores();
		auto& signalSemaphoreInfos = cmd.GetSignalSemaphores();

		std::vector<uint64_t> waitValues;
		std::vector<uint64_t> signalValues;
		std::vector<VkSemaphore> waitSemaphores;
		std::vector<VkSemaphore> signalSemaphores;

		waitSemaphores.reserve(waitSemaphoreInfos.size());
		signalSemaphores.reserve(signalSemaphoreInfos.size());

		for (auto& info : waitSemaphoreInfos)
		{
			if (info.bTimelineSemaphore)
				waitValues.push_back(info.semaphoreValue);
			waitSemaphores.push_back(info.semaphore);
		}
		for (auto& info : signalSemaphoreInfos)
		{
			if (info.bTimelineSemaphore)
				signalValues.push_back(info.semaphoreValue);
			signalSemaphores.push_back(info.semaphore);
		}
		
		VkTimelineSemaphoreSubmitInfo timelineInfo{};
		timelineInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
		timelineInfo.waitSemaphoreValueCount = waitValues.size();
		timelineInfo.pWaitSemaphoreValues = waitValues.data();
		timelineInfo.signalSemaphoreValueCount = signalValues.size();
		timelineInfo.pSignalSemaphoreValues = signalValues.data();

		VkSubmitInfo sinfo{};
		sinfo.sType = VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO;
		sinfo.pNext = &timelineInfo;
		sinfo.pWaitDstStageMask = &waitStage;
		sinfo.commandBufferCount = 1;
		sinfo.pCommandBuffers = &commandBuffer;
		sinfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size());
		sinfo.pWaitSemaphores = waitSemaphores.data();
		sinfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores.size());
		sinfo.pSignalSemaphores = signalSemaphores.data();

		spinLock.Lock();
		auto result = vkQueueSubmit(queue, 1, &sinfo, fence);
		assert(result == VkResult::VK_SUCCESS);
		spinLock.UnLock();
	}
	void VulkanQueueManager::GetQueueFamilyProperties()
	{
		uint32_t count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(ctx.GetGPU(), &count, nullptr);
		queueFamilyProps.resize(count);
		vkGetPhysicalDeviceQueueFamilyProperties(ctx.GetGPU(), &count, queueFamilyProps.data());
	}
	auto VulkanQueueManager::SelectQueueFamily(VkQueueFlagBits queueType) const -> std::optional<QueueFamily>
	{
		QueueFamily family{};
		int idx = 0;
		for (auto& prop : queueFamilyProps)
		{
			if ((prop.queueFlags & queueType) == queueType)
			{
				family.idx = idx;
				family.queueCount = prop.queueCount;
				return family;
			}
			++idx;
		}
		return {};
	}
	auto VulkanQueueManager::GetSurfaceQueueFamily(VkSurfaceKHR surface) const -> std::optional<QueueFamily>
	{
		QueueFamily family{};
		int idx = 0;
		for (auto& prop : queueFamilyProps)
		{
			VkBool32 support = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(ctx.GetGPU(), idx, surface, &support);
			if (support)
			{
				family.idx = idx;
				family.queueCount = prop.queueCount;
				return family;
			}
			++idx;
		}
		return {};
	}
}//namespace