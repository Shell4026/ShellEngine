#include "VulkanSurface.h"

#include "../Window/Window.h"

#include <cassert>
#if _WIN32
#include <Windows.h>
#endif

namespace sh::render
{
	VulkanSurface::VulkanSurface(sh::window::Window* window) :
		window(window), surface(nullptr), instance(nullptr)
	{
	}

	VulkanSurface::~VulkanSurface()
	{
		DestroySurface();
	}

	auto VulkanSurface::CreateSurface(VkInstance instance) -> VkResult
	{
		assert(window);
		this->instance = instance;
#if _WIN32
		VkWin32SurfaceCreateInfoKHR createInfo{};
		createInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = window->GetNativeHandle();
		createInfo.hinstance = GetModuleHandleW(nullptr);
		createInfo.pNext = nullptr;

		return vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface);
#elif __linux__
		VkXlibSurfaceCreateInfoKHR createInfo{};
		createInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
		createInfo.dpy = winHandle.first;
		createInfo.window = winHandle.second;
		createInfo.pNext = nullptr;

		return vkCreateXlibSurfaceKHR(instance, &createInfo, nullptr, &surface);
#endif
		static_assert(true, "Not Supported OS");
		return VkResult::VK_ERROR_UNKNOWN;
	}

	void VulkanSurface::DestroySurface()
	{
		if (surface)
		{
			vkDestroySurfaceKHR(instance, surface, nullptr);
			surface = nullptr;
		}
	}
}