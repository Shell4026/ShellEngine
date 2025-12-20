#pragma once
#include "../Export.h"
#include "../RenderTarget.h"
#include "VulkanConfig.h"
#include "VulkanImageBuffer.h"

#include "Core/NonCopyable.h"

#include <vector>

namespace sh::window
{
	class Window;
}

namespace sh::render::vk 
{
	class VulkanContext;
	class VulkanSwapChain : public core::INonCopyable
	{
	public:
		struct SwapChainSupportDetails 
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkPresentModeKHR> presentModes;
			std::vector<VkSurfaceFormatKHR> formats;

			SwapChainSupportDetails() = default;
			SwapChainSupportDetails(SwapChainSupportDetails&& other) noexcept :
				capabilities(other.capabilities),
				presentModes(std::move(other.presentModes)), 
				formats(std::move(other.formats))
			{}
		};
	public:
		SH_RENDER_API VulkanSwapChain(const VulkanContext& context);
		SH_RENDER_API VulkanSwapChain(VulkanSwapChain&& other) noexcept;
		SH_RENDER_API ~VulkanSwapChain();

		SH_RENDER_API void CreateSurface(const sh::window::Window& window);
		SH_RENDER_API void DestroySurface();

		SH_RENDER_API void CreateSwapChain(uint8_t graphicsQueueIdx, uint8_t surfaceQueueIdx, bool bVsync);
		SH_RENDER_API void DestroySwapChain();

		SH_RENDER_API auto IsSwapChainSupport(VkPhysicalDevice gpu) -> bool;

		SH_RENDER_API auto GetSurface() const -> const VkSurfaceKHR { return surface; }
		SH_RENDER_API auto GetSwapChain() const -> const VkSwapchainKHR { return swapChain; }
		SH_RENDER_API auto GetSwapChainDetail() const -> const SwapChainSupportDetails& { return details; }
		SH_RENDER_API auto GetSwapChainSize() const -> VkExtent2D { return swapChainSize; }
		SH_RENDER_API auto GetSwapChainImageFormat() const -> VkFormat { return swapChainImageFormat; }
		SH_RENDER_API auto GetSwapChainImages() const -> const std::vector<VulkanImageBuffer>& { return swapChainImages; }
		SH_RENDER_API auto GetSwapChainImages() -> std::vector<VulkanImageBuffer>& { return swapChainImages; }
		SH_RENDER_API auto GetSwapChainMSAAImages() const -> const std::vector<VulkanImageBuffer>& { return swapChainImagesMSAA; }
		SH_RENDER_API auto GetSwapChainMSAAImages() -> std::vector<VulkanImageBuffer>& { return swapChainImagesMSAA; }
		SH_RENDER_API auto GetSwapChainDepthImages() const -> const std::vector<VulkanImageBuffer>& { return swapChainImagesDepth; }
		SH_RENDER_API auto GetSwapChainDepthImages() -> std::vector<VulkanImageBuffer>& { return swapChainImagesDepth; }
		SH_RENDER_API auto GetSwapChainImageCount() const -> uint32_t { return swapChainImageCount; }
		SH_RENDER_API auto GetRenderTargetLayout() const -> const RenderTargetLayout& { return rtLayout; }
	private:
		void QuerySwapChainDetails(VkPhysicalDevice gpu);
		auto SelectSurfaceFormat() -> VkSurfaceFormatKHR;
		auto SelectPresentMode(bool bVsync) -> VkPresentModeKHR;
	private:
		const VulkanContext& context;

		VkSurfaceKHR surface;
		VkSwapchainKHR swapChain;
		std::vector<VulkanImageBuffer> swapChainImagesMSAA;
		std::vector<VulkanImageBuffer> swapChainImages;
		std::vector<VulkanImageBuffer> swapChainImagesDepth;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainSize;

		SwapChainSupportDetails details;

		uint32_t swapChainImageCount;

		RenderTargetLayout rtLayout;
	};
}