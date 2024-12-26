#include "pch.h"
#include "VulkanImpl/VulkanSwapChain.h"

#include "Core/Logger.h"

#include "Window/Window.h"

#include <cassert>
#if _WIN32
#include <Windows.h>
#endif

namespace sh::render::vk
{
	VulkanSwapChain::SwapChainSupportDetails::SwapChainSupportDetails()
	{
	}

	VulkanSwapChain::SwapChainSupportDetails::~SwapChainSupportDetails()
	{
	}

	VulkanSwapChain::VulkanSwapChain() :
		device(nullptr), instance(nullptr), gpu(nullptr),
		swapChain(nullptr), surface(nullptr), details(), 
		swapChainImageCount(1)
	{
	}

	VulkanSwapChain::VulkanSwapChain(VulkanSwapChain&& other) noexcept :
		device(other.device), instance(other.instance), gpu(other.gpu),
		swapChain(other.swapChain), surface(other.surface),
		swapChainImages(std::move(other.swapChainImages)), swapChainImageViews(std::move(other.swapChainImageViews)), 
		swapChainImageCount(other.swapChainImageCount)
	{
		other.swapChain = nullptr;
		other.surface = nullptr;
	}

	VulkanSwapChain::~VulkanSwapChain()
	{
		if (device != nullptr)
			DestroySwapChain();
		if (surface != nullptr)
			DestroySurface();
	}

	SH_RENDER_API void VulkanSwapChain::SetContext(VkInstance instance, VkDevice device, VkPhysicalDevice gpu)
	{
		this->instance = instance;
		this->device = device;
		this->gpu = gpu;
	}

	SH_RENDER_API void VulkanSwapChain::CreateSurface(const sh::window::Window& window)
	{
		assert(instance != nullptr);
#if _WIN32
		VkWin32SurfaceCreateInfoKHR createInfo{};
		createInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = window.GetNativeHandle();
		createInfo.hinstance = GetModuleHandleW(nullptr);
		createInfo.pNext = nullptr;

		auto result = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface);
#elif __linux__

		VkXlibSurfaceCreateInfoKHR createInfo{};
		createInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
		createInfo.dpy = window.GetNativeHandle().first;
		createInfo.window = window.GetNativeHandle().second;
		createInfo.pNext = nullptr;

		auto result = vkCreateXlibSurfaceKHR(instance, &createInfo, nullptr, &surface);
#else
		static_assert(false, "Not Supported OS");
#endif
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			throw std::runtime_error{ std::string{"vkCreateWin32SurfaceKHR(): "} + string_VkResult(result) };
	}
	SH_RENDER_API void VulkanSwapChain::DestroySurface()
	{
		if (surface != nullptr)
		{
			vkDestroySurfaceKHR(instance, surface, nullptr);
			surface = nullptr;
		}
	}

	void VulkanSwapChain::QuerySwapChainDetails(VkPhysicalDevice gpu)
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

	auto VulkanSwapChain::SelectSurfaceFormat() -> VkSurfaceFormatKHR
	{
		assert(!details.formats.empty());
		for (auto& format : details.formats)
		{
			if (format.format == VkFormat::VK_FORMAT_R8G8B8A8_SRGB && format.colorSpace == VkColorSpaceKHR::VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return format;
			}
		}
		return details.formats[0];
	}

	auto VulkanSwapChain::SelectPresentMode(bool bVsync) -> VkPresentModeKHR
	{
		if (bVsync)
			return VK_PRESENT_MODE_FIFO_KHR; // 수직 동기화

		VkPresentModeKHR outputMode = VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR;
		for (const auto& mode : details.presentModes) 
		{
			//삼중 버퍼링
			if (mode == VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR)
			{
				SH_INFO("VK_PRESENT_MODE_MAILBOX_KHR");
				return mode;
			}
			if (mode == VkPresentModeKHR::VK_PRESENT_MODE_IMMEDIATE_KHR)
				outputMode = mode;
		}
		SH_INFO("VK_PRESENT_MODE_IMMEDIATE_KHR");
		return outputMode;
	}

	SH_RENDER_API void VulkanSwapChain::CreateSwapChain(uint8_t graphicsQueueIdx, uint8_t surfaceQueueIdx, bool bVsync)
	{
		VkSwapchainKHR oldSwapchain = swapChain;

		QuerySwapChainDetails(gpu);

		VkSurfaceFormatKHR surfaceFormat = SelectSurfaceFormat();
		VkPresentModeKHR presentMode = SelectPresentMode(bVsync);
		VkExtent2D size = details.capabilities.currentExtent;

		swapChainImageFormat = surfaceFormat.format;
		swapChainSize = size;

		// 스왑 체인에 쓸 이미지 갯수
		// 최소값이면 렌더링할 다른 이미지를 확보하기 전에 드라이버가 내부 작업을 완료할 때까지 기다려야 하는 경우가 있다. 따라서 +1
		swapChainImageCount = details.capabilities.minImageCount + 1;
		if (details.capabilities.maxImageCount > 0 && swapChainImageCount > details.capabilities.maxImageCount)
			swapChainImageCount = details.capabilities.maxImageCount;
		
		VkSwapchainCreateInfoKHR info{};
		info.sType = VkStructureType::VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
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
		info.oldSwapchain = oldSwapchain;

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
		if (result != VkResult::VK_SUCCESS)
			throw std::runtime_error(std::string{ "vkCreateSwapchainKHR()" } + string_VkResult(result));

		if (oldSwapchain != nullptr)
		{
			for (auto imageView : swapChainImageViews)
			{
				vkDestroyImageView(device, imageView, nullptr);
			}
			swapChainImageViews.clear();
			vkDestroySwapchainKHR(device, oldSwapchain, nullptr);
		}

		result = vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, nullptr);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			throw std::runtime_error(std::string{ "vkGetSwapchainImagesKHR()" } + string_VkResult(result));

		swapChainImages.resize(swapChainImageCount);

		result = vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, swapChainImages.data());
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			throw std::runtime_error(std::string{ "vkGetSwapchainImagesKHR()" } + string_VkResult(result));

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
			if (result != VkResult::VK_SUCCESS)
				throw std::runtime_error(std::string{ "vkCreateImageView()" } + string_VkResult(result));
		}
	}

	SH_RENDER_API void VulkanSwapChain::DestroySwapChain()
	{
		assert(device);

		if (swapChain != nullptr)
		{
			for (auto imageView : swapChainImageViews)
			{
				vkDestroyImageView(device, imageView, nullptr);
			}
			swapChainImageViews.clear();
			vkDestroySwapchainKHR(device, swapChain, nullptr);
		}
		swapChain = nullptr;
	}

	SH_RENDER_API bool VulkanSwapChain::IsSwapChainSupport(VkPhysicalDevice gpu)
	{
		QuerySwapChainDetails(gpu);

		return !details.formats.empty() && !details.presentModes.empty();
	}

	SH_RENDER_API auto VulkanSwapChain::GetDevice() const -> const VkDevice
	{
		return device;
	}

	SH_RENDER_API auto VulkanSwapChain::GetSurface() const -> const VkSurfaceKHR
	{
		return surface;
	}

	SH_RENDER_API auto VulkanSwapChain::GetSwapChain() const -> const VkSwapchainKHR
	{
		return swapChain;
	}
	SH_RENDER_API auto VulkanSwapChain::GetSwapChainDetail() const -> const SwapChainSupportDetails&
	{
		return details;
	}
	SH_RENDER_API auto VulkanSwapChain::GetSwapChainSize() const -> const VkExtent2D
	{
		return swapChainSize;
	}
	SH_RENDER_API auto VulkanSwapChain::GetSwapChainImageFormat() const -> const VkFormat
	{
		return swapChainImageFormat;
	}
	SH_RENDER_API auto VulkanSwapChain::GetSwapChainImages() const -> const std::vector<VkImage>&
	{
		return swapChainImages;
	}
	SH_RENDER_API auto VulkanSwapChain::GetSwapChainImageViews() const -> const std::vector<VkImageView>&
	{
		return swapChainImageViews;
	}
	SH_RENDER_API auto VulkanSwapChain::GetSwapChainImageCount() const -> uint32_t
	{
		return swapChainImageCount;
	}
}