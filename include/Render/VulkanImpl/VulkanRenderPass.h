#pragma once
#include "Export.h"
#include "VulkanConfig.h"

#include "Core/NonCopyable.h"

namespace sh::render::vk
{
	class VulkanContext;
	class VulkanRenderPass : public core::INonCopyable
	{
	public:
		struct Config
		{
			VkFormat format;
			VkFormat depthFormat;

			VkSampleCountFlagBits sampleCount = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;

			bool bOffScreen = false;
			bool bUseDepth = true;
			bool bUseStencil = false;
			bool bTransferSrc = false;
			bool bClear = true;

			SH_RENDER_API auto operator==(const Config& other) const -> bool;
		};
		struct ConfigHasher
		{
			SH_RENDER_API auto operator()(const Config& config) const->std::size_t;
		};
	private:
		const VulkanContext& context;
		VkRenderPass renderPass = VK_NULL_HANDLE;
		Config config;

		VkImageLayout finalColorLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
	private:
		auto GetOffScreenSubPassDependency() const -> std::array<VkSubpassDependency, 2>;
		auto GetOnScreenSubPassDependency() const->std::array<VkSubpassDependency, 2>;
	public:
		SH_RENDER_API VulkanRenderPass(const VulkanContext& context);
		SH_RENDER_API VulkanRenderPass(VulkanRenderPass&& other) noexcept;
		SH_RENDER_API ~VulkanRenderPass();
		SH_RENDER_API auto operator=(VulkanRenderPass&& other) noexcept -> VulkanRenderPass&;
		SH_RENDER_API void Create(const Config& config);
		SH_RENDER_API void Clear();
		SH_RENDER_API auto GetVkRenderPass() const -> VkRenderPass;
		SH_RENDER_API auto GetConfig() const -> const Config&;
		SH_RENDER_API auto GetFinalColorLayout() const -> VkImageLayout;
	};
}//namespace