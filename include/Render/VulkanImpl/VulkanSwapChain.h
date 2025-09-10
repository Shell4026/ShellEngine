#pragma once
#include "../Export.h"
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
			
			SH_RENDER_API SwapChainSupportDetails();
			SH_RENDER_API ~SwapChainSupportDetails();
		};
	public:
		SH_RENDER_API VulkanSwapChain(const VulkanContext& context);
		SH_RENDER_API VulkanSwapChain(VulkanSwapChain&& other) noexcept;
		SH_RENDER_API ~VulkanSwapChain();

		SH_RENDER_API void CreateSurface(const sh::window::Window& window);
		SH_RENDER_API void DestroySurface();

		SH_RENDER_API void CreateSwapChain(uint8_t graphicsQueueIdx, uint8_t surfaceQueueIdx, bool bVsync);
		SH_RENDER_API void DestroySwapChain();

		SH_RENDER_API bool IsSwapChainSupport(VkPhysicalDevice gpu);

		SH_RENDER_API auto GetSurface() const -> const VkSurfaceKHR;
		SH_RENDER_API auto GetSwapChain() const -> const VkSwapchainKHR;
		SH_RENDER_API auto GetSwapChainDetail() const -> const SwapChainSupportDetails&;
		SH_RENDER_API auto GetSwapChainSize() const -> const VkExtent2D;
		SH_RENDER_API auto GetSwapChainImageFormat() const -> const VkFormat;
		SH_RENDER_API auto GetSwapChainImages() const -> const std::vector<VulkanImageBuffer>&;
		SH_RENDER_API auto GetSwapChainImages() -> std::vector<VulkanImageBuffer>&;
		SH_RENDER_API auto GetSwapChainImageCount() const -> uint32_t;
	private:
		void QuerySwapChainDetails(VkPhysicalDevice gpu);
		auto SelectSurfaceFormat() -> VkSurfaceFormatKHR;
		auto SelectPresentMode(bool bVsync) -> VkPresentModeKHR;
	private:
		const VulkanContext& context;

		VkSurfaceKHR surface;
		VkSwapchainKHR swapChain;
		std::vector<VulkanImageBuffer> swapChainImages;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainSize;

		SwapChainSupportDetails details;

		uint32_t swapChainImageCount;
	};
}