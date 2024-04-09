#pragma once
#pragma warning(disable: 4251)

#include "Renderer.h"

#if _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif __unix__
#define VK_USE_PLATFORM_XLIB_KHR
#endif

#include <../Core/Singleton.hpp>

#include <vulkan/vulkan.h>
#include <vector>

namespace sh::render {
	class SH_RENDER_API VulkanRenderer :
		public Renderer, 
		public sh::core::Singleton<VulkanRenderer> {
	private:
		sh::window::Window* window;
		sh::window::WinHandle winHandle;

		struct LayerProperties {
			VkLayerProperties properties;
			std::vector<VkExtensionProperties> extensions;
		};

		std::vector<LayerProperties> layers;
		std::vector<LayerProperties> GPULayers;
		std::vector<VkPhysicalDevice> gpus;
		std::vector<VkQueueFamilyProperties> queueFamilyProps;

		VkInstance instance;
		VkDevice device; //논리적 장치
		VkSurfaceKHR surface;

		uint32_t graphicsQueueIndex;
	private:
		auto GetInstanceLayerProperties() -> VkResult;
		auto GetLayerExtensions(LayerProperties& layerProp, VkPhysicalDevice gpu = nullptr) -> VkResult;

		auto CreateInstance() -> VkResult;

		auto CreateSurface() -> VkResult;

		auto GetPhysicalDevices()->VkResult;
		auto GetPhysicalDeviceExtensions(VkPhysicalDevice gpu) -> VkResult;
		auto SelectPhysicalDevice() -> VkPhysicalDevice;
		bool IsDeviceSuitable(VkPhysicalDevice gpu);

		void GetQueueFamilyProperties(VkPhysicalDevice gpu);
		auto SelectQueueFamily() -> int;

		auto CreateDevice(VkPhysicalDevice gpu, uint32_t queueIndex)->VkResult;

		auto CreateCommandPool() -> VkResult;
	public:
		VulkanRenderer();
		~VulkanRenderer();

		bool Init(sh::window::Window& win) override;
	};
}//namespace
