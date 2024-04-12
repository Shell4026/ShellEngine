#pragma once

#include "Export.h"

#include "VulkanConfig.h"

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
		VulkanSurface();
		~VulkanSurface();

		auto CreateSurface(sh::window::Window& window, VkInstance instance) -> VkResult;
		void DestroySurface();
	};
}