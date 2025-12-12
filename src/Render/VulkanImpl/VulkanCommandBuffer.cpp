#include "VulkanCommandBuffer.h"
#include "VulkanContext.h"
#include "Core/Logger.h"

#include <cassert>

namespace sh::render::vk
{
    VulkanCommandBuffer::VulkanCommandBuffer(const VulkanContext& context)
        : context(context)
    {}

    VulkanCommandBuffer::VulkanCommandBuffer(VulkanCommandBuffer&& other) noexcept
        : context(other.context),
        buffer(other.buffer),
        cmdPool(other.cmdPool),
        waitSemaphores(std::move(other.waitSemaphores)),
        signalSemaphores(std::move(other.signalSemaphores)),
        fence(other.fence)
    {
        other.buffer = VK_NULL_HANDLE;
        other.cmdPool = VK_NULL_HANDLE;
        other.fence = VK_NULL_HANDLE;
    }
    VulkanCommandBuffer::~VulkanCommandBuffer()
    {
        Clear();
    }
    SH_RENDER_API auto VulkanCommandBuffer::Create(VkCommandPool pool, VkCommandBufferLevel level) -> VkResult
    {
        Clear();
        cmdPool = pool;

        VkCommandBufferAllocateInfo info{};
        info.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        info.commandPool = cmdPool;
        info.level = level;
        info.commandBufferCount = 1;

        VkResult result = vkAllocateCommandBuffers(context.GetDevice(), &info, &buffer);
        assert(result == VkResult::VK_SUCCESS);
        return result;
    }
    SH_RENDER_API auto VulkanCommandBuffer::Build(const std::function<void()>& recordFn, bool bOneTimeSubmit) -> VkResult
    {
        VkResult result = Begin(bOneTimeSubmit);
        if (result != VkResult::VK_SUCCESS)
            return result;

        recordFn();

        result = End();
        if (result != VkResult::VK_SUCCESS)
            return result;

        return VkResult::VK_SUCCESS;
    }
    SH_RENDER_API auto VulkanCommandBuffer::ResetCommand(VkCommandBufferResetFlags flags) -> VkResult
    {
        if (buffer == VK_NULL_HANDLE) 
            return VkResult::VK_ERROR_UNKNOWN;
        VkResult result = vkResetCommandBuffer(buffer, flags);
        assert(result == VkResult::VK_SUCCESS);
        return result;
    }
    SH_RENDER_API auto VulkanCommandBuffer::GetOrCreateFence() -> VkFence
    {
        if (fence != VK_NULL_HANDLE)
            return fence;

        VkFenceCreateInfo info{};
        info.sType = VkStructureType::VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        info.flags = 0;

        auto result = vkCreateFence(context.GetDevice(), &info, nullptr, &fence);
        assert(result == VK_SUCCESS);
        if (result != VK_SUCCESS)
        {
            SH_ERROR_FORMAT("Failed to create fence: {}", string_VkResult(result));
            fence = VK_NULL_HANDLE;
        }
        return fence;
    }
    SH_RENDER_API void VulkanCommandBuffer::DestroyFence()
    {
        if (fence == VK_NULL_HANDLE)
            return;

        vkDestroyFence(context.GetDevice(), fence, nullptr);
        fence = VK_NULL_HANDLE;
    }
    SH_RENDER_API void VulkanCommandBuffer::ResetSyncObjects()
    {
        waitSemaphores.clear();
        signalSemaphores.clear();
    }
    SH_RENDER_API void VulkanCommandBuffer::SetWaitSemaphores(const std::vector<WaitSemaphore>& waits)
    {
        waitSemaphores = waits;
    }
    SH_RENDER_API void VulkanCommandBuffer::SetSignalSemaphores(const std::vector<SignalSemaphore>& signals)
    {
        signalSemaphores = signals;
    }
    SH_RENDER_API void VulkanCommandBuffer::AddWaitSemaphore(const WaitSemaphore& w)
    {
        waitSemaphores.push_back(w);
    }
    SH_RENDER_API void VulkanCommandBuffer::AddSignalSemaphore(const SignalSemaphore& s)
    {
        signalSemaphores.push_back(s);
    }
    SH_RENDER_API auto VulkanCommandBuffer::GetWaitSemaphores() const -> const std::vector<WaitSemaphore>&
    {
        return waitSemaphores;
    }
    SH_RENDER_API auto VulkanCommandBuffer::GetSignalSemaphores() const -> const std::vector<SignalSemaphore>&
    {
        return signalSemaphores;
    }
    SH_RENDER_API auto VulkanCommandBuffer::GetCommandBuffer() const -> VkCommandBuffer
    {
        return buffer;
    }
    SH_RENDER_API auto VulkanCommandBuffer::GetCommandPool() const -> VkCommandPool
    {
        return cmdPool;
    }
    SH_RENDER_API void VulkanCommandBuffer::Clear()
    {
        ResetSyncObjects();

        if (buffer)
        {
            vkFreeCommandBuffers(context.GetDevice(), cmdPool, 1, &buffer);
            buffer = VK_NULL_HANDLE;
        }

        DestroyFence();
        cmdPool = VK_NULL_HANDLE;
    }

    auto VulkanCommandBuffer::Begin(bool bOneTime) -> VkResult
    {
        if (buffer == VK_NULL_HANDLE) 
            return VkResult::VK_ERROR_UNKNOWN;

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = bOneTime ? VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0;
        beginInfo.pInheritanceInfo = nullptr;

        VkResult result = vkBeginCommandBuffer(buffer, &beginInfo);
        assert(result == VkResult::VK_SUCCESS);
        return result;
    }
    auto VulkanCommandBuffer::End() -> VkResult
    {
        if (buffer == VK_NULL_HANDLE) 
            return VkResult::VK_ERROR_UNKNOWN;
        VkResult result = vkEndCommandBuffer(buffer);
        assert(result == VkResult::VK_SUCCESS);
        return result;
    }
}
