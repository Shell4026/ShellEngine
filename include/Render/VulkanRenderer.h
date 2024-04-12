#pragma once
#pragma warning(disable: 4251)

#include "Renderer.h"

#include <../Core/Singleton.hpp>

#include "VulkanLayer.h"
#include "VulkanSurface.h"

#include <string>
#include <vector>
#include <optional>

namespace sh::render {
	class SH_RENDER_API VulkanRenderer :
		public Renderer, 
		public sh::core::Singleton<VulkanRenderer> {
	private:
		sh::window::Window* window;
		sh::window::WinHandle winHandle;

		VulkanSurface surface;
		VulkanLayer layers;

		VkInstance instance;
		VkDevice device; //논리적 장치
		VkCommandPool cmdPool;

		std::vector<VkPhysicalDevice> gpus;
		std::vector<VkQueueFamilyProperties> queueFamilies;

		VkDebugUtilsMessengerEXT debugMessenger;
		std::string validationLayerName;

		uint32_t graphicsQueueIndex;

		bool bFindValidationLayer : 1;
		const bool bEnableValidationLayers : 1;
	private:
		auto CreateInstance()->VkResult;

		auto CreateDebugInfo()->VkDebugUtilsMessengerCreateInfoEXT;
		void InitDebugMessenger();
		void DestroyDebugMessenger();

		auto GetPhysicalDevices()->VkResult;
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
