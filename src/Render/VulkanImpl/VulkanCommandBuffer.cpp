#include "VulkanCommandBuffer.h"
#include "VulkanContext.h"

#include "Core/Logger.h"

#include <cassert>

namespace sh::render::vk 
{
	VulkanCommandBuffer::VulkanCommandBuffer(const VulkanContext& context) :
		context(context),
		waitStage(0)
	{
	}

	VulkanCommandBuffer::VulkanCommandBuffer(VulkanCommandBuffer&& other) noexcept :
		context(other.context), 
		buffer(other.buffer), cmdPool(other.cmdPool),
		waitStage(other.waitStage), waitSemaphores(std::move(other.waitSemaphores)), signalSemaphores(std::move(other.signalSemaphores))
	{
		other.buffer = nullptr;
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		Clear();
	}

	SH_RENDER_API auto VulkanCommandBuffer::Create(VkCommandPool cmdPool, const VkCommandBufferAllocateInfo* info)->VkResult
	{
		Clear();
		this->cmdPool = cmdPool;

		VkResult result;
		if (info)
		{
			result = vkAllocateCommandBuffers(context.GetDevice(), info, &buffer);
			assert(result == VkResult::VK_SUCCESS);
			return result;
		}

		VkCommandBufferAllocateInfo cmdInfo = {};
		cmdInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdInfo.pNext = nullptr;
		cmdInfo.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdInfo.commandPool = cmdPool;
		cmdInfo.commandBufferCount = 1;

		result = vkAllocateCommandBuffers(context.GetDevice(), &cmdInfo, &buffer);
		assert(result == VkResult::VK_SUCCESS);

		return result;
	}
	SH_RENDER_API auto VulkanCommandBuffer::GetFence() const -> VkFence
	{
		if (fence == nullptr)
		{
			VkFenceCreateInfo info{};
			info.sType = VkStructureType::VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			info.flags = 0;
			auto result = vkCreateFence(context.GetDevice(), &info, nullptr, &fence);
			assert(result == VkResult::VK_SUCCESS);
			if (result != VkResult::VK_SUCCESS)
			{
				SH_ERROR_FORMAT("Failed to create fence! {}", string_VkResult(result));
				return nullptr;
			}
		}
		return fence;
	}

	auto VulkanCommandBuffer::Begin(bool bOnce) -> VkResult
	{
		assert(buffer);

		VkResult result;
		if (!buffer)
			return VkResult::VK_ERROR_UNKNOWN;

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
		if (bOnce)
			beginInfo.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		else
			beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr; //inheritInfo

		result = vkBeginCommandBuffer(buffer, &beginInfo);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			SH_ERROR_FORMAT("Failed to create command buffer! {}", string_VkResult(result));
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
		waitStage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_NONE;
		for (auto i : s)
			waitStage |= i;
	}
	SH_RENDER_API auto VulkanCommandBuffer::GetWaitStage() const -> VkPipelineStageFlags
	{
		return waitStage;
	}

	SH_RENDER_API void VulkanCommandBuffer::SetWaitSemaphore(const SemaphoreInfo& semaphore)
	{
		waitSemaphores.clear();
		waitSemaphores.push_back(semaphore);
	}
	SH_RENDER_API void VulkanCommandBuffer::SetWaitSemaphore(const std::initializer_list<SemaphoreInfo>& s)
	{
		waitSemaphores.clear();
		waitSemaphores.resize(s.size());
		int idx = 0;
		for (auto i : s)
			waitSemaphores[idx++] = i;
	}
	SH_RENDER_API void VulkanCommandBuffer::SetWaitSemaphore(const std::vector<SemaphoreInfo>& semaphores)
	{
		waitSemaphores = semaphores;
	}
	SH_RENDER_API void VulkanCommandBuffer::SetWaitSemaphore(std::vector<SemaphoreInfo>&& semaphores)
	{
		waitSemaphores = std::move(semaphores);
	}
	SH_RENDER_API auto VulkanCommandBuffer::GetWaitSemaphores() const -> const std::vector<SemaphoreInfo>&
	{
		return waitSemaphores;
	}

	SH_RENDER_API void VulkanCommandBuffer::SetSignalSemaphore(const SemaphoreInfo& semaphore)
	{
		signalSemaphores.clear();
		signalSemaphores.push_back(semaphore);
	}
	SH_RENDER_API void VulkanCommandBuffer::SetSignalSemaphore(const std::initializer_list<SemaphoreInfo>& s)
	{
		signalSemaphores.clear();
		signalSemaphores.resize(s.size());
		int idx = 0;
		for (auto i : s)
			signalSemaphores[idx++] = i;
	}
	SH_RENDER_API void VulkanCommandBuffer::SetSignalSemaphore(const std::vector<SemaphoreInfo>& semaphores)
	{
		signalSemaphores = semaphores;
	}
	SH_RENDER_API void VulkanCommandBuffer::SetSignalSemaphore(std::vector<SemaphoreInfo>&& semaphores)
	{
		signalSemaphores = std::move(signalSemaphores);
	}
	SH_RENDER_API auto VulkanCommandBuffer::GetSignalSemaphores() const -> const std::vector<SemaphoreInfo>&
	{
		return signalSemaphores;
	}

	SH_RENDER_API auto VulkanCommandBuffer::Build(const std::function<void()>& commands, bool bUsingOnce) -> VkResult
	{
		VkResult result;
		if (result = Begin(bUsingOnce); result != VkResult::VK_SUCCESS) return result;
		commands();
		if (result = End(); result != VkResult::VK_SUCCESS) return result;

		return VkResult::VK_SUCCESS;
	}

	SH_RENDER_API auto VulkanCommandBuffer::Reset() -> VkResult
	{
		if (buffer != nullptr)
		{
			VkResult result = vkResetCommandBuffer(buffer, VkCommandBufferResetFlagBits::VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
			assert(result == VkResult::VK_SUCCESS);

			return result;
		}

		return VkResult::VK_ERROR_UNKNOWN;
	}
	SH_RENDER_API void VulkanCommandBuffer::ResetSyncObjects()
	{
		waitStage = 0;
		waitSemaphores.clear();
		signalSemaphores.clear();
	}

	SH_RENDER_API void VulkanCommandBuffer::Clear()
	{
		ResetSyncObjects();
		if (buffer != nullptr)
		{
			vkFreeCommandBuffers(context.GetDevice(), cmdPool, 1, &buffer);
			buffer = nullptr;
		}
		if (fence != nullptr)
		{
			vkDestroyFence(context.GetDevice(), fence, nullptr);
			fence = nullptr;
		}
	}

	SH_RENDER_API auto VulkanCommandBuffer::GetCommandBuffer() const -> VkCommandBuffer
	{
		return buffer;
	}
}