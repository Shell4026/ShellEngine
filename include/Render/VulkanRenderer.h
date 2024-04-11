#pragma once
#pragma warning(disable: 4251)

#include "Renderer.h"

#include <../Core/Singleton.hpp>
#include "VulkanSurface.h"

#include <string>
#include <vector>
#include <optional>
#include <memory>

namespace sh::render {
	class SH_RENDER_API VulkanRenderer :
		public Renderer, 
		public sh::core::Singleton<VulkanRenderer> {
	private:
		sh::window::Window* window;
		sh::window::WinHandle winHandle;

		std::unique_ptr<VulkanSurface> surface;

		struct LayerProperties {
			VkLayerProperties properties;
			std::vector<VkExtensionProperties> extensions;
		};

		std::vector<LayerProperties> layers;
		std::vector<LayerProperties> GPULayers;
		std::vector<VkPhysicalDevice> gpus;
		std::vector<VkQueueFamilyProperties> queueFamilies;

		VkInstance instance;

		VkDebugUtilsMessengerCreateInfoEXT debugInfo;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkDevice device; //논리적 장치

		uint32_t graphicsQueueIndex;

		VkCommandPool cmdPool;

		std::string validationLayerName;
		bool bFindValidationLayer : 1;
		const bool bEnableValidationLayers : 1;
	private:
		auto GetInstanceLayerProperties() -> VkResult;
		auto GetLayerExtensions(LayerProperties& layerProp, VkPhysicalDevice gpu = nullptr) -> VkResult;

		auto CreateInstance() -> VkResult;
		void InitDebugMessenger();
		void DestroyDebugMessenger();

		auto GetPhysicalDevices()->VkResult;
		auto GetPhysicalDeviceExtensions(VkPhysicalDevice gpu) -> VkResult;
		auto SelectPhysicalDevice() -> VkPhysicalDevice;
		bool IsDeviceSuitable(VkPhysicalDevice gpu);

		void GetQueueFamilyProperties(VkPhysicalDevice gpu);
		auto SelectQueueFamily() -> std::optional<int>;

		auto CreateDevice(VkPhysicalDevice gpu, uint32_t queueIndex)->VkResult;

		auto CreateCommandPool(uint32_t queue) -> VkResult;
		auto ResetCommandPool(uint32_t queue) -> VkResult;
	public:
		VulkanRenderer();
		~VulkanRenderer();

		bool Init(sh::window::Window& win) override;
	};
}//namespace
