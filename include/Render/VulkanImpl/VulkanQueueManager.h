#pragma once
#include "../Export.h"
#include "VulkanConfig.h"
#include "Core/NonCopyable.h"

#include <vector>
#include <optional>
#include <mutex>

namespace sh::render::vk
{
    class VulkanContext;
    class VulkanCommandBuffer;

    class VulkanQueueManager : public core::INonCopyable
    {
    public:
        enum class Role { Graphics, Present, Transfer };

        struct Family
        {
            uint32_t index = UINT32_MAX;
            uint32_t availableCount = 0; // VkQueueFamilyProperties::queueCount
            uint32_t requestedCount = 0; // vkCreateDevice에 요청할 개수
        };

        struct Selection
        {
            Family graphics;
            Family present;
            Family transfer;

            // 역할별 queue index(패밀리 내부 인덱스)
            uint32_t graphicsQIndex = 0;
            uint32_t presentQIndex = 0;
            uint32_t transferQIndex = 0;
        };

    public:
        SH_RENDER_API VulkanQueueManager(const VulkanContext& ctx);
        SH_RENDER_API VulkanQueueManager(VulkanQueueManager&& other) noexcept;
        SH_RENDER_API ~VulkanQueueManager() = default;

        SH_RENDER_API void QueryFamilies(VkSurfaceKHR surface);
        SH_RENDER_API auto BuildQueueCreateInfos() -> std::vector<VkDeviceQueueCreateInfo>;
        SH_RENDER_API void FetchQueues();

        SH_RENDER_API auto GetQueue(Role role) const -> VkQueue;
        SH_RENDER_API auto GetFamilyIndex(Role role) const -> uint32_t;

        // 스레드 안전 제출
        SH_RENDER_API void Submit(Role role, const VulkanCommandBuffer& cmd, VkFence fence = VK_NULL_HANDLE);

    private:
        void LoadFamilyProps();
        auto PickGraphicsFamily() const -> std::optional<Family>;
        auto PickPresentFamily(VkSurfaceKHR surface, uint32_t preferFamily) const -> std::optional<Family>;
        auto PickTransferFamily(uint32_t avoidFamily) const -> std::optional<Family>;

        void ComputeRequestedCountsAndIndices();
        auto ClampRequested(uint32_t want, uint32_t avail) const ->uint32_t;

        auto GetMutexForRole(Role role) const -> std::mutex*;

    private:
        const VulkanContext& ctx;

        std::vector<VkQueueFamilyProperties> props;
        Selection selection;

        VkQueue graphicsQueue = VK_NULL_HANDLE;
        VkQueue presentQueue = VK_NULL_HANDLE;
        VkQueue transferQueue = VK_NULL_HANDLE;

        // 동일 VkQueue를 공유하면 동일 mutex를 공유하도록 포인터로 묶는다
        mutable std::mutex graphicsMtx;
        mutable std::mutex presentMtx;
        mutable std::mutex transferMtx;

        mutable std::mutex* graphicsLock = &graphicsMtx;
        mutable std::mutex* presentLock = &presentMtx;
        mutable std::mutex* transferLock = &transferMtx;

        // BuildQueueCreateInfos에서 사용하는 우선순위 배열 (가변 길이)
        std::vector<float> graphicsPriorities;
        std::vector<float> presentPriorities;
        std::vector<float> transferPriorities;

        bool bFamiliesReady = false;
        bool bQueuesReady = false;
    };
}//namespace