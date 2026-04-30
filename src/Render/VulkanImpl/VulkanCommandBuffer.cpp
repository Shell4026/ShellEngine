#include "VulkanCommandBuffer.h"
#include "VulkanContext.h"
#include "VulkanImageBuffer.h"
#include "VulkanBuffer.h"
#include "VulkanDescriptorSet.h"
#include "VulkanPipelineManager.h"
#include "VulkanComputePipeline.h"
#include "VulkanComputePipelineManager.h"
#include "VulkanSwapChain.h"
#include "VulkanShaderPass.h"
#include "VulkanCameraBuffers.h"
#include "VulkanVertexBuffer.h"
#include "VulkanSkinnedVertexBuffer.h"
#include "Render/RenderTexture.h"
#include "Render/ComputeShader.h"
#include "Render/Shader.h"
#include "Render/Material.h"
#include "Render/Mesh.h"
#include "Render/SkinnedMesh.h"
#include "Render/Drawable.h"
#include "Render/ShaderPass.h"
#include "Render/Camera.h"
#include "Render/RenderData.h"

#include "Core/Reflection.hpp"
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

    SH_RENDER_API void VulkanCommandBuffer::Begin(bool bOnetime)
    {
        if (buffer == VK_NULL_HANDLE)
            return;

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = bOnetime ? VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0;
        beginInfo.pInheritanceInfo = nullptr;

        VkResult result = vkBeginCommandBuffer(buffer, &beginInfo);
        assert(result == VkResult::VK_SUCCESS);
        if (result != VkResult::VK_SUCCESS)
        {
            const std::string err = fmt::format("Failed vkBeginCommandBuffer: {}", string_VkResult(result));
            SH_ERROR(err);
            throw std::runtime_error{ err };
        }
    }
    SH_RENDER_API void VulkanCommandBuffer::End()
    {
        if (buffer == VK_NULL_HANDLE)
            return;

        if (bBeginRender)
        {
            static PFN_vkCmdEndRenderingKHR pfnEnd = (PFN_vkCmdEndRenderingKHR)vkGetDeviceProcAddr(context.GetDevice(), "vkCmdEndRenderingKHR");
            pfnEnd(buffer);
            bBeginRender = false;
        }
        renderState = RenderState{};

        VkResult result = vkEndCommandBuffer(buffer);
        assert(result == VkResult::VK_SUCCESS);
        assert(result == VkResult::VK_SUCCESS);
        if (result != VkResult::VK_SUCCESS)
        {
            const std::string err = fmt::format("Failed vkEndCommandBuffer: {}", string_VkResult(result));
            SH_ERROR(err);
            throw std::runtime_error{ err };
        }
    }

    SH_RENDER_API void VulkanCommandBuffer::Blit(RenderTexture& src, int x, int y, IBuffer& dst)
    {
        // 배리어는 렌더 그래프에서 지정해줌

        VulkanImageBuffer& imgBuffer = *static_cast<VulkanImageBuffer*>(src.GetTextureBuffer());
        VulkanBuffer& buffer = static_cast<VulkanBuffer&>(dst);

        VkBufferImageCopy cpy{};
        cpy.bufferOffset = 0;
        cpy.bufferRowLength = 0;
        cpy.bufferImageHeight = 0;
        cpy.imageSubresource.aspectMask = imgBuffer.GetAspect();
        cpy.imageSubresource.mipLevel = 0;
        cpy.imageSubresource.baseArrayLayer = 0;
        cpy.imageSubresource.layerCount = 1;
        cpy.imageOffset = { x, y, 0 };
        cpy.imageExtent = { 1, 1, 1 };

        vkCmdCopyImageToBuffer(this->buffer, imgBuffer.GetImage(), VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer.GetBuffer(), 1, &cpy);
    }

    SH_RENDER_API void VulkanCommandBuffer::Dispatch(const ComputeShader& shader, uint32_t x, uint32_t y, uint32_t z)
    {
        const VulkanDescriptorSet* const descSet = static_cast<VulkanDescriptorSet*>(shader.GetShaderBinding());
        if (descSet == nullptr)
            return;
        const VulkanComputePipeline& pipeline = context.GetComputePipelineManager().GetOrCreatePipeline(shader);
        VkDescriptorSet vkDescriptorSet = descSet->GetVkDescriptorSet();

        vkCmdBindPipeline(buffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.GetPipeline());
        vkCmdBindDescriptorSets(buffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.GetPipelineLayout(), 0, 1, &vkDescriptorSet, 0, nullptr);
        vkCmdDispatch(buffer, x, y, z);
    }

    SH_RENDER_API void VulkanCommandBuffer::SetRenderTarget(const RenderTarget& renderTarget, bool bClearColor, bool bClearDepth, bool bStoreColor, bool bStoreDepth)
    {
        if (bBeginRender)
        {
            static PFN_vkCmdEndRenderingKHR pfnEndPrev = (PFN_vkCmdEndRenderingKHR)vkGetDeviceProcAddr(context.GetDevice(), "vkCmdEndRenderingKHR");
            pfnEndPrev(buffer);
            bBeginRender = false;
        }

        const bool bDepthOnly = renderTarget.target == nullptr ? false : renderTarget.target->IsDepthOnly();
        const VulkanImageBuffer* colorImg = nullptr;
        const VulkanImageBuffer* colorMSAAImg = nullptr;
        const VulkanImageBuffer* depthImg = nullptr;
        RenderTargetLayout rtLayout{};
        if (renderTarget.target != nullptr)
        {
            colorImg = bDepthOnly ? nullptr : static_cast<VulkanImageBuffer*>(renderTarget.target->GetTextureBuffer());
            colorMSAAImg = static_cast<VulkanImageBuffer*>(renderTarget.target->GetMSAABuffer());
            depthImg = static_cast<VulkanImageBuffer*>(renderTarget.target->GetDepthBuffer());
            rtLayout = renderTarget.target->GetLayout();
        }
        else
        {
            colorImg = &context.GetSwapChain().GetSwapChainImages()[renderTarget.frameIndex];
            colorMSAAImg = &context.GetSwapChain().GetSwapChainMSAAImages()[renderTarget.frameIndex];
            depthImg = &context.GetSwapChain().GetSwapChainDepthImages()[renderTarget.frameIndex];
            rtLayout = context.GetSwapChain().GetRenderTargetLayout();
        }
        const VulkanImageBuffer* const sizeRef = colorImg != nullptr ? colorImg : depthImg;
        const uint32_t width = sizeRef->GetWidth();
        const uint32_t height = sizeRef->GetHeight();
        const bool bHasDepth = (depthImg != nullptr);

        VkRenderingAttachmentInfoKHR colorAttachment{};
        if (!bDepthOnly)
        {
            colorAttachment.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            const bool bMSAA = (colorMSAAImg != nullptr);
            colorAttachment.imageView = bMSAA ? colorMSAAImg->GetImageView() : colorImg->GetImageView();
            colorAttachment.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorAttachment.loadOp = bClearColor ? VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR : VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;
            colorAttachment.storeOp = bStoreColor ? VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE : VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.clearValue.color = { { 0.f, 0.f, 0.f, 1.f } };
            if (bMSAA)
            {
                colorAttachment.resolveImageView = colorImg->GetImageView();
                colorAttachment.resolveImageLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                colorAttachment.resolveMode = VkResolveModeFlagBitsKHR::VK_RESOLVE_MODE_AVERAGE_BIT_KHR;
            }
        }
        VkRenderingAttachmentInfoKHR depthAttachment{};
        if (bHasDepth)
        {
            depthAttachment.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            depthAttachment.imageView = depthImg->GetImageView();
            depthAttachment.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthAttachment.loadOp = bClearDepth ? VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR : VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_LOAD;
            depthAttachment.storeOp = bStoreDepth ? VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE : VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.clearValue.depthStencil = { 1.0f, 0 };
        }

        VkRenderingInfoKHR renderingInfo{};
        renderingInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
        renderingInfo.colorAttachmentCount = bDepthOnly ? 0 : 1;
        renderingInfo.pColorAttachments = bDepthOnly ? nullptr : &colorAttachment;
        renderingInfo.pDepthAttachment = bHasDepth ? &depthAttachment : nullptr;
        // depth only는 스텐실 지원x
        renderingInfo.pStencilAttachment = (bHasDepth && !bDepthOnly) ? &depthAttachment : nullptr;
        renderingInfo.renderArea = { { 0, 0 }, { width, height } };
        renderingInfo.layerCount = 1;

        static PFN_vkCmdBeginRenderingKHR pfnBegin = (PFN_vkCmdBeginRenderingKHR)vkGetDeviceProcAddr(context.GetDevice(), "vkCmdBeginRenderingKHR");
        pfnBegin(buffer, &renderingInfo);
        bBeginRender = true;

        renderState = RenderState{};
        renderState.layout = rtLayout;
        renderState.camera = renderTarget.camera;
        renderState.targetWidth = width;
        renderState.targetHeight = height;
        renderState.bDepthOnly = bDepthOnly;
    }

    SH_RENDER_API void VulkanCommandBuffer::SetViewport(int x, int y, int width, int height)
    {
        VkViewport viewport{};
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        viewport.x = static_cast<float>(x);
        viewport.y = static_cast<float>(y);
        viewport.width = static_cast<float>(width);
        viewport.height = static_cast<float>(height);

        vkCmdSetViewport(buffer, 0, 1, &viewport);
    }

    SH_RENDER_API void VulkanCommandBuffer::SetScissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        VkRect2D rect;
        rect.offset.x = x;
        rect.offset.y = y;
        rect.extent.width = width;
        rect.extent.height = height;

        vkCmdSetScissor(buffer, 0, 1, &rect);
    }

    SH_RENDER_API void VulkanCommandBuffer::DrawMesh(const Drawable& drawable, core::Name passName)
    {
        if (!bBeginRender || renderState.camera == nullptr)
            return;

        const Material& mat = *drawable.GetMaterial();
        const Mesh& mesh = *drawable.GetMesh();
        const Mesh::Topology topology = drawable.GetTopology();
        const bool bSkinned = drawable.IsSkinnedMesh();
        const Shader* const shader = mat.GetShader();
        if (!core::IsValid(shader))
            return;

        const std::vector<std::reference_wrapper<ShaderPass>>* passes = shader->GetShaderPasses(passName);
        if (passes == nullptr)
            return;

        std::optional<uint32_t> cameraOffsetOpt = VulkanCameraBuffers::GetInstance()->GetDynamicOffset(*renderState.camera);
        if (!cameraOffsetOpt.has_value())
            return;
        const uint32_t cameraOffset = cameraOffsetOpt.value();

        for (const ShaderPass& pass : *passes)
        {
            if (pass.IsPendingKill())
                continue;

            const VulkanShaderPass& vkPass = static_cast<const VulkanShaderPass&>(pass);
            VkPipelineLayout pipelineLayout = vkPass.GetPipelineLayout();
            const std::vector<uint8_t>* constantData = mat.GetConstantData(pass);
            const uint32_t setSize = static_cast<uint32_t>(vkPass.GetSetCount());
            VulkanPipelineManager::PipelineHandle handle =
                context.GetPipelineManager().GetOrCreatePipelineHandle(vkPass, renderState.layout, topology, bSkinned, constantData);

            if (handle.index != renderState.lastPipelineIdx || handle.generation != renderState.lastPipelineGen)
            {
                context.GetPipelineManager().BindPipeline(buffer, handle);
                renderState.lastPipelineIdx = handle.index;
                renderState.lastPipelineGen = handle.generation;
            }
            // set 0=Camera, 1=Object, 2=Material

            if (setSize > 0)
                BindCameraSet(mat, pass, pipelineLayout, cameraOffset);
            if (setSize > 2)
                BindMaterialSet(mat, pass, pipelineLayout);
            if (setSize > 1)
                BindObjectSet(drawable, pass, pipelineLayout);

            if (pass.HasConstantUniform())
            {
                vkCmdPushConstants(buffer, pipelineLayout,
                    VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT,
                    0, sizeof(glm::mat4),
                    &drawable.GetModelMatrix(core::ThreadType::Render));
            }

            BindMesh(mesh, drawable.GetSubMeshIndex(), bSkinned);
        }
    }
    SH_RENDER_API void VulkanCommandBuffer::DrawMeshBatch(const std::vector<const Drawable*>& drawables, core::Name passName)
    {
        if (drawables.empty())
            return;

        const Material& mat = *drawables.front()->GetMaterial();
        const Mesh::Topology topology = drawables.front()->GetTopology();
        const bool bSkinned = drawables.front()->IsSkinnedMesh();

        const Shader* const shader = mat.GetShader();
        if (!core::IsValid(shader))
            return;

        const std::vector<std::reference_wrapper<ShaderPass>>* passes = shader->GetShaderPasses(passName);
        if (passes == nullptr)
            return;

        std::optional<uint32_t> cameraOffsetOpt = VulkanCameraBuffers::GetInstance()->GetDynamicOffset(*renderState.camera);
        if (!cameraOffsetOpt.has_value())
            return;
        const uint32_t cameraOffset = cameraOffsetOpt.value();

        for (const ShaderPass& pass : *passes)
        {
            if (pass.IsPendingKill())
                continue;

            const VulkanShaderPass& vkPass = static_cast<const VulkanShaderPass&>(pass);
            VkPipelineLayout pipelineLayout = vkPass.GetPipelineLayout();
            const std::vector<uint8_t>* constantData = mat.GetConstantData(pass);
            const uint32_t setSize = static_cast<uint32_t>(vkPass.GetSetCount());
            VulkanPipelineManager::PipelineHandle handle =
                context.GetPipelineManager().GetOrCreatePipelineHandle(vkPass, renderState.layout, topology, bSkinned, constantData);

            if (handle.index != renderState.lastPipelineIdx || handle.generation != renderState.lastPipelineGen)
            {
                context.GetPipelineManager().BindPipeline(buffer, handle);
                renderState.lastPipelineIdx = handle.index;
                renderState.lastPipelineGen = handle.generation;
            }
            // set 0=Camera, 1=Object, 2=Material

            if (setSize > 0)
                BindCameraSet(mat, pass, pipelineLayout, cameraOffset);
            if (setSize > 2)
                BindMaterialSet(mat, pass, pipelineLayout);

            for (const Drawable* drawable : drawables)
            {
                if (setSize > 1)
                    BindObjectSet(*drawable, pass, pipelineLayout);

                if (pass.HasConstantUniform())
                {
                    vkCmdPushConstants(buffer, pipelineLayout,
                        VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT,
                        0, sizeof(glm::mat4),
                        &drawable->GetModelMatrix(core::ThreadType::Render));
                }

                const Mesh& mesh = *drawable->GetMesh();
                BindMesh(mesh, drawable->GetSubMeshIndex(), bSkinned);
            }
        }
    }
    SH_RENDER_API void VulkanCommandBuffer::EmitBarrier(const std::vector<BarrierInfo>& barriers)
    {
        for (const BarrierInfo& barrier : barriers)
        {
            VulkanImageBuffer* imgBuffer = nullptr;
            VulkanImageBuffer* msaaBuffer = nullptr;

            if (barrier.target.index() == 0)
            {
                imgBuffer = (static_cast<VulkanImageBuffer*>(std::get<0>(barrier.target)->GetTextureBuffer()));
                msaaBuffer = (static_cast<VulkanImageBuffer*>(std::get<0>(barrier.target)->GetMSAABuffer()));
            }
            else
            {
                const uint32_t imgIdx = std::get<1>(barrier.target);
                imgBuffer = &context.GetSwapChain().GetSwapChainImages()[imgIdx];
                msaaBuffer = &context.GetSwapChain().GetSwapChainMSAAImages()[imgIdx];
            }

            VkImageLayout srcLayout, dstLayout;
            VkPipelineStageFlags srcStage, dstStage;
            VkAccessFlags srcAccess, dstAccess;

            auto mapUsageFn =
                [&](
                    ResourceUsage u,
                    VkImageLayout& layout,
                    VkPipelineStageFlags& stage,
                    VkAccessFlags& access)
                {
                    switch (u)
                    {
                    case ResourceUsage::Undefined:
                        layout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
                        stage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                        access = 0;
                        return;
                    case ResourceUsage::ColorAttachment:
                        layout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                        stage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                        access = VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
                        return;
                    case ResourceUsage::SampledRead:
                        layout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        stage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
                        access = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;
                        return;
                    case ResourceUsage::Present:
                        layout = VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                        stage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
                        access = 0;
                        return;
                    case ResourceUsage::TransferSrc:
                        layout = VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                        stage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT;
                        access = VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT;
                        return;
                    case ResourceUsage::DepthStencilAttachment:
                        layout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                        stage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                            VkPipelineStageFlagBits::VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                        access = VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                            VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
                        return;
                    case ResourceUsage::DepthStencilSampledRead:
                        layout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                        stage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                        access = VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;
                        return;
                    }
                };

            mapUsageFn(barrier.lastUsage, srcLayout, srcStage, srcAccess);
            mapUsageFn(barrier.curUsage, dstLayout, dstStage, dstAccess);

            VulkanImageBuffer::BarrierCommand(buffer, *imgBuffer, srcLayout, dstLayout, srcStage, dstStage, srcAccess, dstAccess);

            if (msaaBuffer != nullptr)
            {
                // MSAA 버퍼를 다른 용도로 쓸 일이 없을듯?
                //if (barrier.curUsage != ResourceUsage::Present)
                    //VulkanImageBuffer::BarrierCommand(cmd.GetCommandBuffer(), *msaaBuffer, srcLayout, dstLayout, srcStage, dstStage, srcAccess, dstAccess);
            }
        }
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
        Begin(bOneTimeSubmit);
        recordFn();
        End();
        return VkResult::VK_SUCCESS;
    }
    SH_RENDER_API void VulkanCommandBuffer::Reset()
    {
        if (buffer == VK_NULL_HANDLE)
            return;
        VkResult result = vkResetCommandBuffer(buffer, 0);
        assert(result == VkResult::VK_SUCCESS);

        bBeginRender = false;
        renderState = RenderState{};
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
        if (fence != VK_NULL_HANDLE)
        {
            vkResetFences(context.GetDevice(), 1, &fence);
        }
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

    void VulkanCommandBuffer::BindCameraSet(const Material& mat, const ShaderPass& pass, VkPipelineLayout pipelineLayout, uint32_t cameraOffset)
    {
        VulkanDescriptorSet* const cameraUBO = static_cast<VulkanDescriptorSet*>(
            mat.GetMaterialData().GetShaderBinding(pass, UniformStructLayout::Usage::Camera));
        VkDescriptorSet cameraSet = cameraUBO ? cameraUBO->GetVkDescriptorSet() : context.GetEmptyDescriptorSet();
        const uint32_t dynamicCount = cameraUBO ? 1 : 0;
        vkCmdBindDescriptorSets(buffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
            static_cast<uint32_t>(UniformStructLayout::Usage::Camera),
            1, &cameraSet, dynamicCount, &cameraOffset);
    }
    void VulkanCommandBuffer::BindMaterialSet(const Material& mat, const ShaderPass& pass, VkPipelineLayout pipelineLayout)
    {
        VulkanDescriptorSet* const matUBO = static_cast<VulkanDescriptorSet*>(
            mat.GetMaterialData().GetShaderBinding(pass, UniformStructLayout::Usage::Material));
        VkDescriptorSet matSet = matUBO ? matUBO->GetVkDescriptorSet() : context.GetEmptyDescriptorSet();
        vkCmdBindDescriptorSets(buffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
            static_cast<uint32_t>(UniformStructLayout::Usage::Material),
            1, &matSet, 0, nullptr);
    }
    void VulkanCommandBuffer::BindObjectSet(const Drawable& drawable, const ShaderPass& pass, VkPipelineLayout pipelineLayout)
    {
        VulkanDescriptorSet* const objUBO = static_cast<VulkanDescriptorSet*>(
            drawable.GetMaterialData().GetShaderBinding(pass, UniformStructLayout::Usage::Object));
        VkDescriptorSet objSet = objUBO ? objUBO->GetVkDescriptorSet() : context.GetEmptyDescriptorSet();
        vkCmdBindDescriptorSets(buffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
            static_cast<uint32_t>(UniformStructLayout::Usage::Object),
            1, &objSet, 0, nullptr);
    }
    void VulkanCommandBuffer::BindMesh(const Mesh& mesh, uint32_t subMeshIdx, bool bSkinned)
    {
        uint32_t indexCount = 0;
        uint32_t firstIndex = 0;
        const std::vector<SubMesh>& subMeshes = mesh.GetSubMeshes();
        const uint32_t subMeshIndex = subMeshIdx;
        if (subMeshIndex < static_cast<uint32_t>(subMeshes.size()))
        {
            indexCount = static_cast<uint32_t>(subMeshes[subMeshIndex].indexCount);
            firstIndex = static_cast<uint32_t>(subMeshes[subMeshIndex].indexOffset);
        }
        else
        {
            indexCount = static_cast<uint32_t>(mesh.GetIndices().size());
            firstIndex = 0;
        }

        if (bSkinned)
        {
            const SkinnedMesh* const skinned = static_cast<const SkinnedMesh*>(&mesh);
            const VulkanSkinnedVertexBuffer* vkSkinnedVB =
                static_cast<VulkanSkinnedVertexBuffer*>(skinned->GetVertexBuffer());
            VkBuffer buffers[2] = {
                vkSkinnedVB->GetVertexBuffer().GetBuffer(),
                vkSkinnedVB->GetBoneBuffer().GetBuffer()
            };
            VkDeviceSize offsets[2] = { 0, 0 };
            vkCmdBindVertexBuffers(buffer, 0, 2, buffers, offsets);
            vkCmdBindIndexBuffer(buffer, vkSkinnedVB->GetIndexBuffer().GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
        }
        else
        {
            const VulkanVertexBuffer* vkVB = static_cast<VulkanVertexBuffer*>(mesh.GetVertexBuffer());
            VkBuffer vbs[1] = { vkVB->GetVertexBuffer().GetBuffer() };
            VkDeviceSize offsets[1] = { 0 };
            vkCmdBindVertexBuffers(buffer, 0, 1, vbs, offsets);
            vkCmdBindIndexBuffer(buffer, vkVB->GetIndexBuffer().GetBuffer(), 0, VkIndexType::VK_INDEX_TYPE_UINT32);
        }

        vkCmdDrawIndexed(buffer, indexCount, 1, firstIndex, 0, 0);
    }
}//namespace