#pragma once
#include "Export.h"
#include "VulkanConfig.h"

#include "Core/NonCopyable.h"

namespace sh::render::vk
{
	class VulkanContext;
	class VulkanRenderPass : public core::INonCopyable
	{
	private:
		const VulkanContext& context;
		VkRenderPass renderPass = VK_NULL_HANDLE;

		bool bOffscreen = false;
		bool bUseStencil = false;
		bool bTransferSrc = false;
	public:
		struct Config
		{
			VkFormat format;
			VkFormat depthFormat;
			bool bOffScreen = false;
			bool bUseStencil = false;
			bool bTransferSrc = false;

			SH_RENDER_API auto operator==(const Config& other) const -> bool;
		};
		struct ConfigHasher
		{
			SH_RENDER_API auto operator()(const Config& config) const->std::size_t;
		};
	public:
		SH_RENDER_API VulkanRenderPass(const VulkanContext& context);
		SH_RENDER_API VulkanRenderPass(VulkanRenderPass&& other) noexcept;
		SH_RENDER_API ~VulkanRenderPass();
		SH_RENDER_API auto operator=(VulkanRenderPass&& other) noexcept -> VulkanRenderPass&;
		SH_RENDER_API void Create(const Config& config);
		SH_RENDER_API void Clear();
		SH_RENDER_API auto GetVkRenderPass() const -> VkRenderPass;
	};
}//namespace