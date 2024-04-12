#pragma once
#pragma warning(disable: 4251)

#include "Export.h"

#include "VulkanConfig.h"
#include <vector>

namespace sh::window
{
	class Window;
}
namespace sh::render { class VulkanRenderer; }

namespace sh::render::impl {
	class SH_RENDER_API VulkanSurface
	{
		friend VulkanRenderer;
	public:
		struct SwapChainSupportDetails {
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};
	private:
		sh::window::Window* window;

		VkSurfaceKHR surface;
		VkInstance instance;
		VkSwapchainKHR swapChain;
		std::vector<VkImage> swapChainImages;
		std::vector<VkImageView> swapChainImageViews;
		VkFormat swapChainImageFormat;
		VkExtent2D swapChainSize;

		SwapChainSupportDetails details;
	private:
		VulkanSurface();

		void QuerySwapChainDetails(VkPhysicalDevice gpu);

		auto SelectFormat()->VkSurfaceFormatKHR;
		auto SelectPresentMode()->VkPresentModeKHR;
	public:
		~VulkanSurface();

		bool CreateSurface(sh::window::Window& window, VkInstance instance);
		void DestroySurface();

		bool CreateSwapChain(VkDevice device);
		void DestroySwapChain(VkDevice device);
		
		bool IsSwapChainSupport(VkPhysicalDevice gpu);

		auto GetSurface()->VkSurfaceKHR;
		auto GetSwapChainDetail() const -> const SwapChainSupportDetails&;
	};
}