#pragma once

#include "Export.h"
#include "VulkanConfig.h"
#include "VulkanRenderPass.h"
#include "VulkanImageBuffer.h"
#include "Render/Framebuffer.h"

#include "Core/NonCopyable.h"

#include <vector>
#include <memory>

namespace sh::render::vk
{
	class VulkanContext;

	class VulkanFramebuffer : public Framebuffer
	{
	private:
		const VulkanContext& context;

		VkDevice device;
		VkPhysicalDevice gpu;
		VmaAllocator alloc;

		VkFramebuffer framebuffer;
		VkImageView img;
		
		const VulkanRenderPass* renderPass = nullptr;

		std::unique_ptr<VulkanImageBuffer> colorImg; //Only Offscreen
		std::unique_ptr<VulkanImageBuffer> colorImgMSAA; // Offscreen, MSAA
		std::unique_ptr<VulkanImageBuffer> depthImg;

		uint32_t width, height;
	private:
		void CreateDepthBuffer();
	public:
		SH_RENDER_API VulkanFramebuffer(const VulkanContext& context);
		SH_RENDER_API VulkanFramebuffer(VulkanFramebuffer&& other) noexcept;
		SH_RENDER_API ~VulkanFramebuffer();

		SH_RENDER_API auto operator=(VulkanFramebuffer&& other) noexcept -> VulkanFramebuffer&;

		SH_RENDER_API auto Create(const VulkanRenderPass& renderPass, uint32_t width, uint32_t height, VkImageView img) -> VkResult;
		SH_RENDER_API auto CreateOffScreen(const VulkanRenderPass& renderPass, uint32_t width, uint32_t height) -> VkResult;
		SH_RENDER_API void Clean();
		SH_RENDER_API void TransferImageToBuffer(VkBuffer buffer, int x, int y);

		SH_RENDER_API auto GetRenderPass() const -> const VulkanRenderPass*;
		SH_RENDER_API auto GetVkFramebuffer() const -> VkFramebuffer;

		SH_RENDER_API auto GetColorImg() const -> VulkanImageBuffer*;
		SH_RENDER_API auto GetDepthImg() const -> VulkanImageBuffer*;

		SH_RENDER_API auto GetWidth() const -> uint32_t override;
		SH_RENDER_API auto GetHeight() const->uint32_t override;
	};
}