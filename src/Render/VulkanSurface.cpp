﻿#include "VulkanSurface.h"

#include "../Window/Window.h"

#include <cassert>
#if _WIN32
#include <Windows.h>
#endif

namespace sh::render::impl
{
	VulkanSurface::VulkanSurface() :
		window(nullptr), surface(nullptr), instance(nullptr), details()
	{
	}

	VulkanSurface::~VulkanSurface()
	{
		DestroySurface();
	}

	void VulkanSurface::QuerySwapChainDetails(VkPhysicalDevice gpu)
	{
		assert(surface);

		VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &details.capabilities);
		assert(result == VkResult::VK_SUCCESS);

		uint32_t formatCount;
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, nullptr);
		assert(result == VkResult::VK_SUCCESS);

		details.formats.resize(formatCount);
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, details.formats.data());
		assert(result == VkResult::VK_SUCCESS);

		uint32_t presentCount;
		result = vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentCount, nullptr);
		assert(result == VkResult::VK_SUCCESS);

		details.presentModes.resize(formatCount);
		result = vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentCount, details.presentModes.data());
		assert(result == VkResult::VK_SUCCESS);
	}

	bool VulkanSurface::CreateSurface(sh::window::Window& window, VkInstance instance)
	{
		this->window = &window;
		this->instance = instance;

#if _WIN32
		VkWin32SurfaceCreateInfoKHR createInfo{};
		createInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = window.GetNativeHandle();
		createInfo.hinstance = GetModuleHandleW(nullptr);
		createInfo.pNext = nullptr;

		return vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface) == VkResult::VK_SUCCESS;
#elif __linux__

		VkXlibSurfaceCreateInfoKHR createInfo{};
		createInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
		createInfo.dpy = window.GetNativeHandle().first;
		createInfo.window = window.GetNativeHandle().second;
		createInfo.pNext = nullptr;

		return vkCreateXlibSurfaceKHR(instance, &createInfo, nullptr, &surface) == VkResult::VK_SUCCESS;
#else
		static_assert(true, "Not Supported OS");
		return VkResult::VK_ERROR_UNKNOWN;
#endif
	}

	void VulkanSurface::DestroySurface()
	{
		if (surface)
		{
			vkDestroySurfaceKHR(instance, surface, nullptr);
			surface = nullptr;
		}
	}

	auto VulkanSurface::SelectFormat() -> VkSurfaceFormatKHR
	{
		for (auto& format : details.formats)
		{
			if (format.format == VkFormat::VK_FORMAT_R8G8B8A8_SRGB && format.colorSpace == VkColorSpaceKHR::VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return format;
			}
		}
		return details.formats[0];
	}

	auto VulkanSurface::SelectPresentMode() -> VkPresentModeKHR
	{
		for (const auto& mode : details.presentModes) {
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return mode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	bool VulkanSurface::CreateSwapChain(VkDevice device)
	{
		VkSurfaceFormatKHR surfaceFormat = SelectFormat();
		VkPresentModeKHR presentMode = SelectPresentMode();
		VkExtent2D size = details.capabilities.currentExtent;

		swapChainImageFormat = surfaceFormat.format;
		swapChainSize = size;

		//스왑 체인에 쓸 이미지 갯수
		//최소값이면 렌더링할 다른 이미지를 확보하기 전에 드라이버가 내부 작업을 완료할 때까지 기다려야 하는 경우가 있다. 따라서 +1
		uint32_t imageCount = details.capabilities.minImageCount + 1;
		if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount)
			imageCount = details.capabilities.maxImageCount;
		
		VkSwapchainCreateInfoKHR info{};
		info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		info.surface = surface;
		info.minImageCount = imageCount;
		info.imageFormat = swapChainImageFormat;
		info.imageColorSpace = surfaceFormat.colorSpace;
		info.imageExtent = swapChainSize;
		info.imageArrayLayers = 1;
		info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //VK_IMAGE_USAGE_TRANSFER_DST_BIT = 오프스크린 렌더링
		info.preTransform = details.capabilities.currentTransform;
		info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		info.presentMode = presentMode;
		info.clipped = true;
		info.oldSwapchain = nullptr;

		VkResult result = vkCreateSwapchainKHR(device, &info, nullptr, &swapChain);
		assert(result == VkResult::VK_SUCCESS);
		if (result) return false;

		result = vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		assert(result == VkResult::VK_SUCCESS);
		if (result) return false;

		swapChainImages.resize(imageCount);

		result = vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
		assert(result == VkResult::VK_SUCCESS);
		if (result) return false;

		return true;
	}

	void VulkanSurface::DestroySwapChain(VkDevice device)
	{
		assert(device);
		if (swapChain)
		{
			vkDestroySwapchainKHR(device, swapChain, nullptr);
			swapChain = nullptr;
		}
	}

	bool VulkanSurface::IsSwapChainSupport(VkPhysicalDevice gpu)
	{
		QuerySwapChainDetails(gpu);

		return !details.formats.empty() && !details.presentModes.empty();
	}

	auto VulkanSurface::GetSurface() -> VkSurfaceKHR
	{
		return surface;
	}
	auto VulkanSurface::GetSwapChainDetail() const -> const SwapChainSupportDetails&
	{
		return details;
	}
}