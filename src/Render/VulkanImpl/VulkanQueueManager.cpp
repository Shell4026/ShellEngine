#include "VulkanQueueManager.h"
#include "VulkanContext.h"
#include "VulkanCommandBuffer.h"

#include <cassert>
#include <unordered_map>

namespace sh::render::vk
{
    VulkanQueueManager::VulkanQueueManager(const VulkanContext& ctx)
        : ctx(ctx)
    {
        LoadFamilyProps();
    }

    VulkanQueueManager::VulkanQueueManager(VulkanQueueManager&& other) noexcept
        : ctx(other.ctx),
        props(std::move(other.props)),
        selection(other.selection),
        graphicsQueue(other.graphicsQueue),
        presentQueue(other.presentQueue),
        transferQueue(other.transferQueue),
        graphicsPriorities(std::move(other.graphicsPriorities)),
        presentPriorities(std::move(other.presentPriorities)),
        transferPriorities(std::move(other.transferPriorities)),
        bFamiliesReady(other.bFamiliesReady),
        bQueuesReady(other.bQueuesReady)
    {
        other.graphicsQueue = VK_NULL_HANDLE;
        other.presentQueue = VK_NULL_HANDLE;
        other.transferQueue = VK_NULL_HANDLE;
        other.bFamiliesReady = false;
        other.bQueuesReady = false;

        // mutex 포인터는 move 이후 재결합이 필요 → FetchQueues에서 재설정하게 둔다
        graphicsLock = &graphicsMtx;
        presentLock = &presentMtx;
        transferLock = &transferMtx;
    }
    SH_RENDER_API void VulkanQueueManager::QueryFamilies(VkSurfaceKHR surface)
    {
        auto g = PickGraphicsFamily();
        if (!g.has_value()) 
            throw std::runtime_error("No graphics queue family found");

        auto p = PickPresentFamily(surface, g->index);
        if (!p.has_value()) 
            throw std::runtime_error("No present queue family found");

        auto t = PickTransferFamily(g->index);
        if (!t.has_value()) 
            throw std::runtime_error("No transfer queue family found");

        selection.graphics = *g;
        selection.present = *p;
        selection.transfer = *t;

        ComputeRequestedCountsAndIndices();

        bFamiliesReady = true;
        bQueuesReady = false;
    }
    SH_RENDER_API auto VulkanQueueManager::BuildQueueCreateInfos() -> std::vector<VkDeviceQueueCreateInfo>
    {
        if (!bFamiliesReady) 
            throw std::runtime_error("Call QueryFamilies() before BuildQueueCreateInfos()");

        struct Req 
        { 
            uint32_t family; 
            uint32_t count; 
        };
        std::vector<Req> reqs;

        auto addReqFn = 
            [&](const Family& family)
            {
                for (auto& r : reqs)
                    if (r.family == family.index)
                    { 
                        r.count = std::max(r.count, family.requestedCount);
                        return; 
                    }
                reqs.push_back({ family.index, family.requestedCount });
            };

        addReqFn(selection.graphics);
        addReqFn(selection.present);
        addReqFn(selection.transfer);

        std::vector<VkDeviceQueueCreateInfo> infos;
        infos.reserve(reqs.size());

        graphicsPriorities.clear();
        presentPriorities.clear();
        transferPriorities.clear();

        struct Blob 
        { 
            uint32_t family; 
            std::vector<float>* prios; 
        };
        std::vector<Blob> blobs;
        blobs.reserve(reqs.size());

        auto pickStorageFn = 
            [&](uint32_t family) -> std::vector<float>&
            {
                if (family == selection.graphics.index) 
                    return graphicsPriorities;
                if (family == selection.present.index)  
                    return presentPriorities;
                return transferPriorities;
            };

        for (auto& req : reqs)
        {
            auto& prios = pickStorageFn(req.family);
            prios.assign(req.count, 1.0f);

            VkDeviceQueueCreateInfo q{};
            q.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            q.queueFamilyIndex = req.family;
            q.queueCount = req.count;
            q.pQueuePriorities = prios.data();

            infos.push_back(q);
        }

        return infos;
    }
    SH_RENDER_API void VulkanQueueManager::FetchQueues()
    {
        if (!bFamiliesReady) 
            throw std::runtime_error("Call QueryFamilies() before FetchQueues()");

        VkDevice device = ctx.GetDevice();

        vkGetDeviceQueue(device, selection.graphics.index, selection.graphicsQIndex, &graphicsQueue);
        vkGetDeviceQueue(device, selection.present.index, selection.presentQIndex, &presentQueue);
        vkGetDeviceQueue(device, selection.transfer.index, selection.transferQIndex, &transferQueue);

        // 동일 VkQueue면 락도 공유
        graphicsLock = &graphicsMtx;
        presentLock = &presentMtx;
        transferLock = &transferMtx;

        if (presentQueue == graphicsQueue)
            presentLock = graphicsLock;
        if (transferQueue == graphicsQueue)
            transferLock = graphicsLock;
        if (transferQueue == presentQueue)
            transferLock = presentLock;

        bQueuesReady = true;
    }
    SH_RENDER_API auto VulkanQueueManager::GetQueue(Role role) const -> VkQueue
    {
        if (!bQueuesReady) 
            return VK_NULL_HANDLE;
        switch (role)
        {
        case Role::Graphics: 
            return graphicsQueue;
        case Role::Present:  
            return presentQueue;
        case Role::Transfer: 
            return transferQueue;
        }
        return VK_NULL_HANDLE;
    }
    SH_RENDER_API auto VulkanQueueManager::GetFamilyIndex(Role role) const -> uint32_t
    {
        switch (role)
        {
        case Role::Graphics: 
            return selection.graphics.index;
        case Role::Present:  
            return selection.present.index;
        case Role::Transfer: 
            return selection.transfer.index;
        }
        return UINT32_MAX;
    }
    SH_RENDER_API void VulkanQueueManager::Submit(Role role, const VulkanCommandBuffer& cmd, VkFence fence)
    {
        if (!bQueuesReady) 
            throw std::runtime_error("Call FetchQueues() before Submit()");
        VkQueue q = GetQueue(role);
        if (q == VK_NULL_HANDLE) 
            throw std::runtime_error("Queue is null");

        const auto& waits = cmd.GetWaitSemaphores();
        const auto& signals = cmd.GetSignalSemaphores();
        VkCommandBuffer cb = cmd.GetCommandBuffer();

        std::vector<VkSemaphore> waitSems;
        std::vector<VkPipelineStageFlags> waitStages;
        std::vector<uint64_t> waitValues;

        waitSems.reserve(waits.size());
        waitStages.reserve(waits.size());
        waitValues.reserve(waits.size());

        bool bHasTimeline = false;

        for (auto& wait : waits)
        {
            waitSems.push_back(wait.semaphore);
            waitStages.push_back(wait.stageMask);

            waitValues.push_back(wait.value);
            bHasTimeline |= wait.value > 0;
        }

        std::vector<VkSemaphore> signalSems;
        std::vector<uint64_t> signalValues;

        signalSems.reserve(signals.size());
        signalValues.reserve(signals.size());

        for (auto& signal : signals)
        {
            signalSems.push_back(signal.semaphore);
            signalValues.push_back(signal.value);
            bHasTimeline |= signal.value > 0;
        }

        VkTimelineSemaphoreSubmitInfo timelineInfo{};
        timelineInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
        timelineInfo.waitSemaphoreValueCount = static_cast<uint32_t>(waitValues.size());
        timelineInfo.pWaitSemaphoreValues = waitValues.empty() ? nullptr : waitValues.data();
        timelineInfo.signalSemaphoreValueCount = static_cast<uint32_t>(signalValues.size());
        timelineInfo.pSignalSemaphoreValues = signalValues.empty() ? nullptr : signalValues.data();

        VkSubmitInfo si{};
        si.sType = VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO;
        si.pNext = bHasTimeline ? &timelineInfo : nullptr;

        si.waitSemaphoreCount = static_cast<uint32_t>(waitSems.size());
        si.pWaitSemaphores = waitSems.empty() ? nullptr : waitSems.data();
        si.pWaitDstStageMask = waitStages.empty() ? nullptr : waitStages.data();

        si.commandBufferCount = 1;
        si.pCommandBuffers = &cb;

        si.signalSemaphoreCount = (uint32_t)signalSems.size();
        si.pSignalSemaphores = signalSems.empty() ? nullptr : signalSems.data();

        std::mutex* muPtr = GetMutexForRole(role);
        std::lock_guard<std::mutex> lock(*muPtr);

        VkResult result = vkQueueSubmit(q, 1, &si, fence);
        assert(result == VkResult::VK_SUCCESS);
        if (result != VkResult::VK_SUCCESS)
            throw std::runtime_error("vkQueueSubmit failed");
    }

    void VulkanQueueManager::LoadFamilyProps()
    {
        uint32_t count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(ctx.GetGPU(), &count, nullptr);
        props.resize(count);
        vkGetPhysicalDeviceQueueFamilyProperties(ctx.GetGPU(), &count, props.data());
    }
    auto VulkanQueueManager::PickGraphicsFamily() const -> std::optional<VulkanQueueManager::Family>
    {
        for (int i = 0; i < props.size(); ++i)
        {
            const auto& p = props[i];
            if (p.queueCount == 0) 
                continue;
            if (p.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
            {
                Family fam{};
                fam.index = i;
                fam.availableCount = p.queueCount;
                return fam;
            }
        }
        return {};
    }
    auto VulkanQueueManager::PickPresentFamily(VkSurfaceKHR surface, uint32_t preferFamily) const -> std::optional<VulkanQueueManager::Family>
    {
        // graphics와 같은 패밀리가 present 지원하면 최우선
        if (preferFamily != UINT32_MAX)
        {
            VkBool32 support = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(ctx.GetGPU(), preferFamily, surface, &support);
            if (support)
            {
                Family fam{};
                fam.index = preferFamily;
                fam.availableCount = props[preferFamily].queueCount;
                return fam;
            }
        }

        for (uint32_t i = 0; i < (uint32_t)props.size(); ++i)
        {
            if (props[i].queueCount == 0) 
                continue;

            VkBool32 support = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(ctx.GetGPU(), i, surface, &support);
            if (support)
            {
                Family fam{};
                fam.index = i;
                fam.availableCount = props[i].queueCount;
                return fam;
            }
        }
        return {};
    }
    auto VulkanQueueManager::PickTransferFamily(uint32_t avoidFamily) const -> std::optional<VulkanQueueManager::Family>
    {
        // 전용 transfer (TRANSFER 있고 GRAPHICS/COMPUTE 없는 패밀리) 우선
        for (int i = 0; i < props.size(); ++i)
        {
            const auto& p = props[i];
            if (p.queueCount == 0) 
                continue;
            const bool hasTransfer = (p.queueFlags & VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT) != 0;
            const bool hasGraphics = (p.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT) != 0;
            const bool hasCompute = (p.queueFlags & VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT) != 0;
            if (hasTransfer && !hasGraphics && !hasCompute)
            {
                Family fam{};
                fam.index = i;
                fam.availableCount = p.queueCount;
                return fam;
            }
        }

        // 그래픽과 다른 transfer 가능 패밀리 선호
        for (int i = 0; i < props.size(); ++i)
        {
            const auto& p = props[i];
            if (p.queueCount == 0) 
                continue;
            if (!(p.queueFlags & VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT)) 
                continue;
            if (i == avoidFamily) 
                continue;

            Family fam{};
            fam.index = i;
            fam.availableCount = p.queueCount;
            return fam;
        }

        // 일치하는 게 하나도 없는 경우 - 그래픽 패밀리(대부분 transfer 포함)
        if (avoidFamily != UINT32_MAX)
        {
            const auto& p = props[avoidFamily];
            if (p.queueCount > 0 && (p.queueFlags & VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT))
            {
                Family fam{};
                fam.index = avoidFamily;
                fam.availableCount = p.queueCount;
                return fam;
            }
        }

        return {};
    }
    void VulkanQueueManager::ComputeRequestedCountsAndIndices()
    {
        selection.graphics.requestedCount = 1;
        selection.present.requestedCount = 1;
        selection.transfer.requestedCount = 1;

        // 같은 패밀리 공유 시, 가능한 한 큐 인덱스를 분리해서 받도록 요청 개수 증가
        if (selection.present.index == selection.graphics.index)
            selection.graphics.requestedCount += 1;

        if (selection.transfer.index == selection.graphics.index)
            selection.graphics.requestedCount += 1;

        if (selection.transfer.index == selection.present.index && selection.transfer.index != selection.graphics.index)
            selection.present.requestedCount += 1;

        selection.graphics.requestedCount = ClampRequested(selection.graphics.requestedCount, selection.graphics.availableCount);
        selection.present.requestedCount = ClampRequested(selection.present.requestedCount, selection.present.availableCount);
        selection.transfer.requestedCount = ClampRequested(selection.transfer.requestedCount, selection.transfer.availableCount);

        // 역할별 queue index 결정
        selection.graphicsQIndex = 0;

        // present
        if (selection.present.index == selection.graphics.index)
        {
            selection.presentQIndex = (selection.graphics.requestedCount >= 2) ? 1 : 0;
        }
        else
        {
            selection.presentQIndex = 0;
        }

        // transfer
        if (selection.transfer.index == selection.graphics.index)
        {
            // graphics/present가 이미 0/1을 썼을 가능성이 있으니 가능한 인덱스를 고름
            uint32_t want = 1;
            if (selection.present.index == selection.graphics.index && selection.presentQIndex == 1)
                want = 2;
            selection.transferQIndex = (selection.graphics.requestedCount >= (want + 1)) ? want : 0;
        }
        else if (selection.transfer.index == selection.present.index)
        {
            selection.transferQIndex = (selection.present.requestedCount >= 2) ? 1 : 0;
        }
        else
        {
            selection.transferQIndex = 0;
        }
    }
    uint32_t VulkanQueueManager::ClampRequested(uint32_t want, uint32_t avail) const
    {
        if (avail == 0) 
            return 0;
        return (want > avail) ? avail : want;
    }
    auto VulkanQueueManager::GetMutexForRole(Role role) const -> std::mutex*
    {
        switch (role)
        {
        case Role::Graphics: 
            return graphicsLock;
        case Role::Present:  
            return presentLock;
        case Role::Transfer: 
            return transferLock;
        }
        return graphicsLock;
    }
}//namespace