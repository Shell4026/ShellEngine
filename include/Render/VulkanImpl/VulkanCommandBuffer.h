#pragma once
#include "Render/Export.h"
#include "Render/CommandBuffer.h"
#include "Render/RenderData.h"
#include "VulkanConfig.h"
#include "Core/NonCopyable.h"

#include <vector>
#include <functional>

namespace sh::render
{
    class Camera;
    class Mesh;
    struct RenderView;
}
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
        SH_RENDER_API void Reset() override;

        SH_RENDER_API void Blit(RenderTexture& src, int x, int y, IBuffer& dst) override;
        SH_RENDER_API void Dispatch(const ComputeShader& shader, uint32_t x, uint32_t y, uint32_t z) override;
        SH_RENDER_API void SetRenderData(const RenderData& renderData, bool bClearColor = true, bool bClearDepth = true, bool bStoreColor = false, bool bStoreDepth = false) override;
        SH_RENDER_API void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
        SH_RENDER_API void SetScissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
        SH_RENDER_API void DrawMesh(const Drawable& drawable, core::Name passName, std::size_t viewerIdx = 0) override;
        SH_RENDER_API void DrawMeshBatch(const std::vector<const Drawable*>& drawables, core::Name passName, std::size_t viewerIdx = 0) override;
        SH_RENDER_API void EmitBarrier(const std::vector<BarrierInfo>& barriers) override;

        SH_RENDER_API auto GetOrCreateFence() -> VkFence;
        SH_RENDER_API void DestroyFence();

        SH_RENDER_API void ResetSyncObjects();

        SH_RENDER_API void SetWaitSemaphores(const std::vector<WaitSemaphore>& waits);
        SH_RENDER_API void SetSignalSemaphores(const std::vector<SignalSemaphore>& signals);

        SH_RENDER_API void AddWaitSemaphore(const WaitSemaphore& w);
        SH_RENDER_API void AddSignalSemaphore(const SignalSemaphore& s);

        SH_RENDER_API auto GetWaitSemaphores() const -> const std::vector<WaitSemaphore>& { return waitSemaphores; }
        SH_RENDER_API auto GetSignalSemaphores() const -> const std::vector<SignalSemaphore>& { return signalSemaphores; }
        SH_RENDER_API auto GetCommandBuffer() const -> VkCommandBuffer { return buffer; }
        SH_RENDER_API auto GetCommandPool() const -> VkCommandPool { return cmdPool; }

        SH_RENDER_API void Clear();
    protected:
        auto GetRenderCall() const -> uint32_t override { return renderCall; }
    private:
        void BindCameraSet(const Material& mat, const ShaderPass& pass, VkPipelineLayout pipelineLayout, uint32_t cameraOffset);
        void BindMaterialSet(const Material& mat, const ShaderPass& pass, VkPipelineLayout pipelineLayout);
        void BindObjectSet(const Drawable& drawable, const ShaderPass& pass, VkPipelineLayout pipelineLayout);
        void BindMesh(const Mesh& mesh, uint32_t subMeshIdx, bool bSkinned);
    private:
        struct RenderState
        {
            RenderTargetLayout layout{};
            const RenderData* renderData = nullptr;
            uint32_t targetWidth = 0;
            uint32_t targetHeight = 0;
           
            uint32_t lastPipelineIdx = 0xffffffff;
            uint32_t lastPipelineGen = 0xffffffff;
            bool bDepthOnly = false;
        } renderState;

        const VulkanContext& context;
        VkCommandBuffer buffer = VK_NULL_HANDLE;
        VkCommandPool cmdPool = VK_NULL_HANDLE;

        std::vector<WaitSemaphore> waitSemaphores;
        std::vector<SignalSemaphore> signalSemaphores;

        VkFence fence = VK_NULL_HANDLE;

        uint32_t renderCall = 0;

        bool bBeginRender = false;
    };
}//namespace