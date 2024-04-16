#pragma once

#include "Export.h"

#include "VulkanConfig.h"
#include <vector>
#include <iostream>
namespace sh::window
{
	class Window;
}

namespace sh::render::impl {
	class VulkanSurface
	{
	public:
		struct SwapChainSupportDetails {
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;

			SwapChainSupportDetails(){}
			~SwapChainSupportDetails(){}
		};
	private:
		sh::window::Window* window;
		VkDevice device;

		VkSurfaceKHR surface;
		VkInstance instance;
		VkSwapchainKHR swapChain;
		std::vector<VkImage> swapChainImages;
		std::vector<VkImageView> swapChainImageViews;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainSize;

		SwapChainSupportDetails details;
	private:
		void QuerySwapChainDetails(VkPhysicalDevice gpu);

		auto SelectFormat()->VkSurfaceFormatKHR;
		auto SelectPresentMode()->VkPresentModeKHR;
	public:
		SH_RENDER_API VulkanSurface();
		SH_RENDER_API ~VulkanSurface();

		SH_RENDER_API bool CreateSurface(sh::window::Window& window, VkInstance instance);
		SH_RENDER_API void DestroySurface();

		SH_RENDER_API bool CreateSwapChain(VkDevice device);
		SH_RENDER_API void DestroySwapChain(VkDevice device);
		
		SH_RENDER_API bool IsSwapChainSupport(VkPhysicalDevice gpu);

		SH_RENDER_API auto GetDevice() const -> const VkDevice;
		SH_RENDER_API auto GetSurface() const -> const VkSurfaceKHR;

		SH_RENDER_API auto GetSwapChain() const -> const VkSwapchainKHR;
		SH_RENDER_API auto GetSwapChainDetail() const -> const SwapChainSupportDetails&;
		SH_RENDER_API auto GetSwapChainSize() const -> const VkExtent2D;
		SH_RENDER_API auto GetSwapChainImageFormat() const -> const VkFormat;

		SH_RENDER_API auto GetSwapChainImages() const -> const std::vector<VkImage>&;
		SH_RENDER_API auto GetSwapChainImageViews() const -> const std::vector<VkImageView>&;
	};
}