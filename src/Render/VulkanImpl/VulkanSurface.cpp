#include "pch.h"
#include "VulkanImpl/VulkanSurface.h"

#include "Window/Window.h"

#include <cassert>
#if _WIN32
#include <Windows.h>
#endif

namespace sh::render::impl
{
	VulkanSurface::SwapChainSupportDetails::SwapChainSupportDetails()
	{
	}

	VulkanSurface::SwapChainSupportDetails::~SwapChainSupportDetails()
	{
	}

	VulkanSurface::VulkanSurface() :
		window(nullptr), device(nullptr), instance(nullptr),
		swapChain(nullptr), surface(nullptr), details(), 
		swapChainImageCount(1)
	{
	}

	VulkanSurface::VulkanSurface(VulkanSurface&& other) noexcept :
		window(other.window), device(other.device), instance(other.instance),
		swapChain(other.swapChain), surface(other.surface),
		swapChainImages(std::move(other.swapChainImages)), swapChainImageViews(std::move(other.swapChainImageViews)), 
		swapChainImageCount(other.swapChainImageCount)
	{
		other.swapChain = nullptr;
		other.surface = nullptr;
	}

	VulkanSurface::~VulkanSurface()
	{
		if(device)
			DestroySwapChain();
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

		details.presentModes.resize(presentCount);
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
		
		for (const auto& mode : details.presentModes) 
		{
			//삼중 버퍼링
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR) 
			{
				std::cout << "Render: PRESENT_MODE_MAILBOX\n";
				return mode;
			}
		}
		//수직 동기화
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	bool VulkanSurface::CreateSwapChain(VkPhysicalDevice gpu, uint32_t graphicsQueueIdx, uint32_t surfaceQueueIdx)
	{
		assert(device);

		QuerySwapChainDetails(gpu);

		VkSurfaceFormatKHR surfaceFormat = SelectFormat();
		VkPresentModeKHR presentMode = SelectPresentMode();
		VkExtent2D size = details.capabilities.currentExtent;

		swapChainImageFormat = surfaceFormat.format;
		swapChainSize = size;

		//스왑 체인에 쓸 이미지 갯수
		//최소값이면 렌더링할 다른 이미지를 확보하기 전에 드라이버가 내부 작업을 완료할 때까지 기다려야 하는 경우가 있다. 따라서 +1
		//버퍼링?
		swapChainImageCount = details.capabilities.minImageCount + 1;
		if (details.capabilities.maxImageCount > 0 && swapChainImageCount > details.capabilities.maxImageCount)
			swapChainImageCount = details.capabilities.maxImageCount;
		
		VkSwapchainCreateInfoKHR info{};
		info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		info.surface = surface;
		info.minImageCount = swapChainImageCount;
		info.imageFormat = swapChainImageFormat;
		info.imageColorSpace = surfaceFormat.colorSpace;
		info.imageExtent = swapChainSize;
		info.imageArrayLayers = 1;
		info.imageUsage = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //VK_IMAGE_USAGE_TRANSFER_DST_BIT = 오프스크린 렌더링
		info.preTransform = details.capabilities.currentTransform;
		info.compositeAlpha =  VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		info.presentMode = presentMode;
		info.clipped = true;
		info.oldSwapchain = nullptr;

		uint32_t queueIdxs[] = { graphicsQueueIdx, surfaceQueueIdx };
		if (graphicsQueueIdx == surfaceQueueIdx)
		{
			info.imageSharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
			info.queueFamilyIndexCount = 0;
			info.pQueueFamilyIndices = nullptr;
		}
		else
		{
			info.imageSharingMode = VkSharingMode::VK_SHARING_MODE_CONCURRENT;
			info.queueFamilyIndexCount = 2;
			info.pQueueFamilyIndices = queueIdxs;
		}


		VkResult result = vkCreateSwapchainKHR(device, &info, nullptr, &swapChain);
		assert(result == VkResult::VK_SUCCESS);
		if (result) return false;

		result = vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, nullptr);
		assert(result == VkResult::VK_SUCCESS);
		if (result) return false;

		swapChainImages.resize(swapChainImageCount);

		result = vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, swapChainImages.data());
		assert(result == VkResult::VK_SUCCESS);
		if (result) return false;

		swapChainImageViews.resize(swapChainImageCount);
		for (size_t i = 0; i < swapChainImages.size(); i++) 
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = swapChainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;
			result = vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]);
			assert(result == VkResult::VK_SUCCESS);
			if (result) return false;
		}

		return true;
	}

	void VulkanSurface::DestroySwapChain()
	{
		assert(device);

		for (auto imageView : swapChainImageViews) {
			vkDestroyImageView(device, imageView, nullptr);
		}
		swapChainImageViews.clear();

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

	void VulkanSurface::SetDevice(VkDevice device)
	{
		this->device = device;
	}

	auto VulkanSurface::GetDevice() const -> const VkDevice
	{
		return device;
	}

	auto VulkanSurface::GetSurface() const -> const VkSurfaceKHR
	{
		return surface;
	}

	auto VulkanSurface::GetSwapChain() const -> const VkSwapchainKHR
	{
		return swapChain;
	}
	auto VulkanSurface::GetSwapChainDetail() const -> const SwapChainSupportDetails&
	{
		return details;
	}
	auto VulkanSurface::GetSwapChainSize() const -> const VkExtent2D
	{
		return swapChainSize;
	}
	auto VulkanSurface::GetSwapChainImageFormat() const -> const VkFormat
	{
		return swapChainImageFormat;
	}
	auto VulkanSurface::GetSwapChainImages() const -> const std::vector<VkImage>&
	{
		return swapChainImages;
	}
	auto VulkanSurface::GetSwapChainImageViews() const -> const std::vector<VkImageView>&
	{
		return swapChainImageViews;
	}
	auto VulkanSurface::GetSwapChainImageCount() const -> uint32_t
	{
		return swapChainImageCount;
	}
}