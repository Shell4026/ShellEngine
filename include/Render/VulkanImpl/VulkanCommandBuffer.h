#pragma once
#include "../Export.h"
#include "../CommandBuffer.h"
#include "VulkanConfig.h"
#include "Core/NonCopyable.h"

#include <vector>
#include <functional>

namespace sh::render::vk
{
    class VulkanContext;

    class VulkanCommandBuffer : public CommandBuffer
    {
    public:
        struct WaitSemaphore
        {
            VkSemaphore semaphore = VK_NULL_HANDLE;
            VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_NONE;
            uint64_t value = 0;
        };

        struct SignalSemaphore
        {
            VkSemaphore semaphore = VK_NULL_HANDLE;
            uint64_t value = 0;
        };

    public:
        SH_RENDER_API VulkanCommandBuffer(const VulkanContext& context);
        SH_RENDER_API VulkanCommandBuffer(VulkanCommandBuffer&& other) noexcept;
        SH_RENDER_API ~VulkanCommandBuffer();

        SH_RENDER_API auto operator=(VulkanCommandBuffer&& other) ->VulkanCommandBuffer& = delete;

        SH_RENDER_API auto Create(VkCommandPool pool, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) -> VkResult;

        /// @brief Begin()과 End()사이에 recordFn이 실행 된다.
        /// @param recordFn 명령 버퍼에 기록할 함수
        /// @param bOneTimeSubmit 일회용인지
        SH_RENDER_API auto Build(const std::function<void()>& recordFn, bool bOneTimeSubmit = true) -> VkResult;
        SH_RENDER_API void Begin(bool bOnetime) override;
        SH_RENDER_API void End() override;

        SH_RENDER_API void Blit(RenderTexture& src, int x, int y, IBuffer& dst) override;

        SH_RENDER_API auto ResetCommand(VkCommandBufferResetFlags flags = 0) -> VkResult;

        SH_RENDER_API auto GetOrCreateFence() -> VkFence;
        SH_RENDER_API void DestroyFence();

        SH_RENDER_API void ResetSyncObjects();

        SH_RENDER_API void SetWaitSemaphores(const std::vector<WaitSemaphore>& waits);
        SH_RENDER_API void SetSignalSemaphores(const std::vector<SignalSemaphore>& signals);

        SH_RENDER_API void AddWaitSemaphore(const WaitSemaphore& w);
        SH_RENDER_API void AddSignalSemaphore(const SignalSemaphore& s);

        SH_RENDER_API auto GetWaitSemaphores() const -> const std::vector<WaitSemaphore>&;
        SH_RENDER_API auto GetSignalSemaphores() const -> const std::vector<SignalSemaphore>&;

        SH_RENDER_API auto GetCommandBuffer() const -> VkCommandBuffer;
        SH_RENDER_API auto GetCommandPool() const -> VkCommandPool;

        SH_RENDER_API void Clear();
    private:
        const VulkanContext& context;
        VkCommandBuffer buffer = VK_NULL_HANDLE;
        VkCommandPool cmdPool = VK_NULL_HANDLE;

        std::vector<WaitSemaphore>   waitSemaphores;
        std::vector<SignalSemaphore> signalSemaphores;

        VkFence fence = VK_NULL_HANDLE;
    };
}//namespace