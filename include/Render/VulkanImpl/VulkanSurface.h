#pragma once

#include "Export.h"
#include "Core/NonCopyable.h"

#include "VulkanConfig.h"
#include <vector>
#include <iostream>
namespace sh::window
{
	class Window;
}

namespace sh::render::impl {
	class VulkanSurface : public core::INonCopyable
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
	private:
		sh::window::Window* window;
		VkDevice device;
		VkInstance instance;

		VkSurfaceKHR surface;
		VkSwapchainKHR swapChain;
		std::vector<VkImage> swapChainImages;
		std::vector<VkImageView> swapChainImageViews;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainSize;

		SwapChainSupportDetails details;

		uint32_t swapChainImageCount;
	private:
		void QuerySwapChainDetails(VkPhysicalDevice gpu);

		auto SelectFormat()->VkSurfaceFormatKHR;
		auto SelectPresentMode()->VkPresentModeKHR;
	public:
		SH_RENDER_API VulkanSurface();
		SH_RENDER_API VulkanSurface(VulkanSurface&& other) noexcept;
		SH_RENDER_API ~VulkanSurface();

		SH_RENDER_API bool CreateSurface(sh::window::Window& window, VkInstance instance);
		SH_RENDER_API void DestroySurface();

		SH_RENDER_API bool CreateSwapChain(VkPhysicalDevice gpu, uint32_t graphicsQueueIdx, uint32_t surfaceQueueIdx);
		SH_RENDER_API void DestroySwapChain();
		
		SH_RENDER_API bool IsSwapChainSupport(VkPhysicalDevice gpu);

		SH_RENDER_API void SetDevice(VkDevice device);

		SH_RENDER_API auto GetDevice() const -> const VkDevice;
		SH_RENDER_API auto GetSurface() const -> const VkSurfaceKHR;
		SH_RENDER_API auto GetSwapChain() const -> const VkSwapchainKHR;
		SH_RENDER_API auto GetSwapChainDetail() const -> const SwapChainSupportDetails&;
		SH_RENDER_API auto GetSwapChainSize() const -> const VkExtent2D;
		SH_RENDER_API auto GetSwapChainImageFormat() const -> const VkFormat;
		SH_RENDER_API auto GetSwapChainImages() const -> const std::vector<VkImage>&;
		SH_RENDER_API auto GetSwapChainImageViews() const -> const std::vector<VkImageView>&;
		SH_RENDER_API auto GetSwapChainImageCount() const -> uint32_t;
	};
}