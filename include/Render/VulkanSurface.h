#pragma once

#include "Export.h"

#if _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif __unix__
#define VK_USE_PLATFORM_XLIB_KHR
#endif
#include <vulkan/vulkan.h>

namespace sh::window
{
	class Window;
}

namespace sh::render {
	class SH_RENDER_API VulkanSurface
	{
	private:
		sh::window::Window* window;

		VkSurfaceKHR surface;
		VkInstance instance;
	public:
		VulkanSurface(sh::window::Window* window);
		~VulkanSurface();

		auto CreateSurface(VkInstance instance) -> VkResult;
		void DestroySurface();
	};
}