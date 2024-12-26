#include "VulkanCommandBuffer.h"

#include <cassert>

namespace sh::render::vk 
{
	VulkanCommandBuffer::VulkanCommandBuffer(VkDevice device, VkCommandPool pool, VkQueueFlagBits queueType) :
		buffer(nullptr), device(device), cmdPool(pool), waitStage(0),
		queueType(queueType)
	{

	}

	VulkanCommandBuffer::VulkanCommandBuffer(VulkanCommandBuffer&& other) noexcept :
		buffer(other.buffer), device(other.device), cmdPool(other.cmdPool), waitStage(other.waitStage),
		waitSemaphores(std::move(other.waitSemaphores)), signalSemaphores(std::move(other.signalSemaphores)),
		queueType(other.queueType)
	{
		other.buffer = nullptr;
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		Clean();
	}

	SH_RENDER_API auto VulkanCommandBuffer::Create(const VkCommandBufferAllocateInfo* info)->VkResult
	{
		Clean();
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

	SH_RENDER_API void VulkanCommandBuffer::SetWaitStage(const std::initializer_list<VkPipelineStageFlagBits> s)
	{
		for (auto i : s)
			waitStage |= i;
	}
	SH_RENDER_API auto VulkanCommandBuffer::GetWaitStage() const -> VkPipelineStageFlags
	{
		return waitStage;
	}

	SH_RENDER_API void VulkanCommandBuffer::SetWaitSemaphore(const std::initializer_list<VkSemaphore> s)
	{
		waitSemaphores.resize(s.size());
		int idx = 0;
		for (auto i : s)
			waitSemaphores[idx++] = i;
	}
	SH_RENDER_API auto VulkanCommandBuffer::GetWaitSemaphore() const -> const std::vector<VkSemaphore>&
	{
		return waitSemaphores;
	}

	SH_RENDER_API void VulkanCommandBuffer::SetSignalSemaphore(const std::initializer_list<VkSemaphore> s)
	{
		signalSemaphores.resize(s.size());
		int idx = 0;
		for (auto i : s)
			signalSemaphores[idx++] = i;
	}
	SH_RENDER_API auto VulkanCommandBuffer::GetSignalSemaphore() const -> const std::vector<VkSemaphore>&
	{
		return signalSemaphores;
	}

	SH_RENDER_API auto VulkanCommandBuffer::Build(const std::function<void()>& commands, VkCommandBufferBeginInfo* beginInfo) -> VkResult
	{
		VkResult result;
		if (result = Begin(beginInfo); result != VkResult::VK_SUCCESS) return result;
		commands();
		if (result = End(); result != VkResult::VK_SUCCESS) return result;

		return VkResult::VK_SUCCESS;
	}

	SH_RENDER_API auto VulkanCommandBuffer::Reset() -> VkResult
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

	SH_RENDER_API void VulkanCommandBuffer::Clean()
	{
		if (buffer)
		{
			vkFreeCommandBuffers(device, cmdPool, 1, &buffer);
			buffer = nullptr;

			waitSemaphores.clear();
			signalSemaphores.clear();
		}
	}

	SH_RENDER_API auto VulkanCommandBuffer::GetCommandBuffer() const -> VkCommandBuffer
	{
		return buffer;
	}

	SH_RENDER_API auto VulkanCommandBuffer::GetQueueType() const -> VkQueueFlagBits
	{
		return queueType;
	}
}