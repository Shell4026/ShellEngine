#pragma once
#include "../Export.h"
#include "../ITextureBuffer.h"
#include "../Formats.hpp"
#include "VulkanCommandBuffer.h"

#include "Core/NonCopyable.h"

#include <cstdint>

namespace sh::render::vk
{
	class VulkanContext;

	class VulkanImageBuffer : public ITextureBuffer
	{
	public:
		SH_RENDER_API VulkanImageBuffer();
		SH_RENDER_API VulkanImageBuffer(VulkanImageBuffer&& other) noexcept;
		SH_RENDER_API ~VulkanImageBuffer();

		SH_RENDER_API auto operator=(VulkanImageBuffer&& other) noexcept -> VulkanImageBuffer&;

		SH_RENDER_API auto Create(const IRenderContext& context, const CreateInfo& info) -> bool override;
		SH_RENDER_API void Clear() override;

		SH_RENDER_API void SetData(const void* data, uint32_t mipLevel) override;

		SH_RENDER_API void CreateFromSwapChain(const IRenderContext& context, VkImage img);

		SH_RENDER_API auto SetAnisotropy(uint32_t aniso) -> VulkanImageBuffer&;
		SH_RENDER_API auto SetFilter(VkFilter filter) -> VulkanImageBuffer&;

		SH_RENDER_API auto GetImage() const -> VkImage { return img; }
		SH_RENDER_API auto GetImageView() const -> VkImageView { return imgView; }
		SH_RENDER_API auto GetSampler() const -> VkSampler { return sampler; }

		SH_RENDER_API auto GetAspect() const -> VkImageAspectFlags { return aspect; }
		SH_RENDER_API auto GetFilter() const -> VkFilter { return filter; }
		SH_RENDER_API auto GetFormat() const -> VkFormat { return format; }
		SH_RENDER_API auto GetSample() const -> VkSampleCountFlagBits { return sample; }
		SH_RENDER_API auto GetWidth() const -> uint32_t { return width; }
		SH_RENDER_API auto GetHeight() const -> uint32_t { return height; }
		SH_RENDER_API auto GetAniso() const -> uint32_t { return aniso; }
		SH_RENDER_API auto GetMipLevel() const -> uint32_t { return mipLevel; }
		
		SH_RENDER_API static void BarrierCommand(
			VkCommandBuffer cmd, 
			const VulkanImageBuffer& img,
			VkImageLayout oldLayout, 
			VkImageLayout newLayout, 
			VkPipelineStageFlags srcStage,
			VkPipelineStageFlags dstStage, 
			VkAccessFlags srcAccess,
			VkAccessFlags dstAccess);
		SH_RENDER_API static auto ConvertTextureFormat(TextureFormat format) -> VkFormat;
	private:
		static auto GetChannelCount(VkFormat format) -> uint32_t;
		static auto IsDepthTexture(TextureFormat format) -> bool;
	private:
		const VulkanContext* ctx = nullptr;

		VkImage img = VK_NULL_HANDLE;
		VkImageView imgView = VK_NULL_HANDLE;
		VmaAllocation imgMem = VK_NULL_HANDLE;
		VkSampler sampler = VK_NULL_HANDLE;

		VkImageAspectFlags aspect = VkImageAspectFlagBits::VK_IMAGE_ASPECT_NONE;
		VkFilter filter = VkFilter::VK_FILTER_LINEAR;
		VkFormat format = VkFormat::VK_FORMAT_UNDEFINED;
		VkSampleCountFlagBits sample = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t channel = 4;
		uint32_t aniso = 0;
		uint32_t mipLevel = 1;

		bool bSwapChainImg = false;
		bool bRenderTarget = false;
	};
}