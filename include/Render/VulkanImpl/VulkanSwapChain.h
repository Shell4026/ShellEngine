#pragma once
#include "Render/Export.h"
#include "Render/RenderData.h"
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

		auto GetSurface() const -> const VkSurfaceKHR { return surface; }
		auto GetSwapChain() const -> const VkSwapchainKHR { return swapChain; }
		auto GetSwapChainDetail() const -> const SwapChainSupportDetails& { return details; }
		auto GetSwapChainSize() const -> VkExtent2D { return swapChainSize; }
		auto GetSwapChainImageFormat() const -> VkFormat { return swapChainImageFormat; }
		auto GetSwapChainImages() const -> const std::vector<VulkanImageBuffer>& { return swapChainImages; }
		auto GetSwapChainImages() -> std::vector<VulkanImageBuffer>& { return swapChainImages; }
		auto GetSwapChainMSAAImages() const -> const std::vector<VulkanImageBuffer>& { return swapChainImagesMSAA; }
		auto GetSwapChainMSAAImages() -> std::vector<VulkanImageBuffer>& { return swapChainImagesMSAA; }
		auto GetSwapChainDepthImages() const -> const std::vector<VulkanImageBuffer>& { return swapChainImagesDepth; }
		auto GetSwapChainDepthImages() -> std::vector<VulkanImageBuffer>& { return swapChainImagesDepth; }
		auto GetSwapChainImageCount() const -> uint32_t { return swapChainImageCount; }
		auto GetRenderTargetLayout() const -> const RenderTargetLayout& { return rtLayout; }
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
}//namespace