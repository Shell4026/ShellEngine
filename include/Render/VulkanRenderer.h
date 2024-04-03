#pragma once

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
		struct LayerProperties {
			VkLayerProperties properties;
			std::vector<VkExtensionProperties> extensions;
		};

		std::vector<LayerProperties> layers;
		std::vector<VkPhysicalDevice> gpus;

		VkInstance instance;

		uint32_t graphicsQueueIndex;
	private:
		auto GetInstanceLayerProperties()->VkResult;
		auto GetLayerExtensions(LayerProperties& layerProp, VkPhysicalDevice gpu = nullptr)->VkResult;
		auto GetPhysicalDevices()->VkResult;
		auto GetPhysicalDeviceExtensions(VkPhysicalDevice gpu)->VkResult;
		bool IsDeviceSuitable(VkPhysicalDevice gpu);

		auto CreateInstance()->VkResult;
		auto CreateDevice()->VkResult;
	public:
		VulkanRenderer();
		~VulkanRenderer();

		bool Init() override;
	};
}//namespace
