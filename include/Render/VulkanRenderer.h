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
	class VulkanRenderer : 
		public Renderer, 
		public sh::core::Singleton<VulkanRenderer> {
	private:
		struct LayerProperties {
			VkLayerProperties properties;
			std::vector<VkExtensionProperties> extensions;
		};

		std::vector<LayerProperties> layers;

		VkInstance instance;
	private:
		auto GetInstanceLayerProperties()->VkResult;
		auto GetExtensionProperties(LayerProperties& layerProp, VkPhysicalDevice* gpu = nullptr)->VkResult;
		auto CreateInstance()->VkResult;
	public:
		~VulkanRenderer();

		bool Init() override;
	};
}//namespace
