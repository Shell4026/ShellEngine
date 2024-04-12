#include "VulkanRenderer.h"

#include <fmt/core.h>
#include "../Core/Util.h"

#include <cassert>

namespace sh::render {
	VulkanRenderer::VulkanRenderer() :
		instance(nullptr), device(nullptr), cmdPool(nullptr), window(nullptr),
		graphicsQueueIndex(0), 
		debugMessenger(nullptr), validationLayerName("VK_LAYER_KHRONOS_validation"),
		bFindValidationLayer(false), bEnableValidationLayers(sh::core::Util::IsDebug())
	{

	}

	VulkanRenderer::~VulkanRenderer()
	{
		if (device)
			vkDestroyDevice(device, nullptr);
		
		surface.DestroySurface();
			
		if (bEnableValidationLayers)
			DestroyDebugMessenger();
			
		//if (instance)
			//vkDestroyInstance(instance, nullptr);
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) 
	{
		fmt::print("Validation layer: {}\n", pCallbackData->pMessage);
		return VK_FALSE;
	}

	auto VulkanRenderer::CreateInstance() -> VkResult
	{
		std::vector<const char*> requestedLayer;

		std::vector<const char*> requestedExtension = 
		{ 
			VK_KHR_SURFACE_EXTENSION_NAME
		};
#if _WIN32
		requestedExtension.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif __linux__
		requestedExtension.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif

		VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
		if (bFindValidationLayer && bEnableValidationLayers)
		{
			requestedLayer.push_back(validationLayerName.c_str());
			requestedExtension.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

			debugInfo = CreateDebugInfo();
		}

		VkApplicationInfo appInfo = {};
		appInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = nullptr;
		appInfo.pApplicationName = "ShellEngine";
		appInfo.applicationVersion = 1;
		appInfo.pEngineName = "ShellEngine";
		appInfo.engineVersion = 1;
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo instanceInfo = {};
		instanceInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceInfo.flags = 0; // 현재 사용되지 않음
		instanceInfo.pApplicationInfo = &appInfo;
		instanceInfo.enabledLayerCount = requestedLayer.size();
		instanceInfo.ppEnabledLayerNames = requestedLayer.data();
		instanceInfo.enabledExtensionCount = requestedExtension.size();
		instanceInfo.ppEnabledExtensionNames = requestedExtension.data();
		if (bEnableValidationLayers)
		{
			instanceInfo.pNext = nullptr;
			instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugInfo;
		}
		else
			instanceInfo.pNext = nullptr;
		//pAllocator = 호스트 메모리의 할당 방법 지정
		VkResult result = vkCreateInstance(&instanceInfo, nullptr, &instance);
		return result;
	}

	auto VulkanRenderer::CreateDebugInfo() -> VkDebugUtilsMessengerCreateInfoEXT
	{
		VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
		debugInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugInfo.messageSeverity =
			VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugInfo.messageType =
			VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugInfo.pfnUserCallback = debugCallback;
		debugInfo.pUserData = nullptr; // Optional

		return debugInfo;
	}

	void VulkanRenderer::InitDebugMessenger()
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		assert(func != nullptr);

		auto info = CreateDebugInfo();

		VkResult result = func(instance, &info, nullptr, &debugMessenger);
		assert(result == VkResult::VK_SUCCESS);
	}

	void VulkanRenderer::DestroyDebugMessenger()
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		assert(func);
		func(instance, debugMessenger, nullptr);
	}

	auto VulkanRenderer::GetPhysicalDevices() -> VkResult
	{
		unsigned int count;
		vkEnumeratePhysicalDevices(instance, &count, nullptr);
		gpus.resize(count);

		return vkEnumeratePhysicalDevices(instance, &count, gpus.data());
	}

	bool VulkanRenderer::IsDeviceSuitable(VkPhysicalDevice gpu)
	{
		if (!gpu)
			return false;

		VkPhysicalDeviceProperties prop;
		vkGetPhysicalDeviceProperties(gpu, &prop);

		VkPhysicalDeviceFeatures feature;
		vkGetPhysicalDeviceFeatures(gpu, &feature);

		return 
			(prop.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) ||
			(prop.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_CPU);
	}

	auto VulkanRenderer::SelectPhysicalDevice() -> VkPhysicalDevice
	{
		for (auto gpu : gpus)
		{
			if (IsDeviceSuitable(gpu))
				return gpu;
		}
		return nullptr;
	}

	void VulkanRenderer::GetQueueFamilyProperties(VkPhysicalDevice gpu)
	{
		uint32_t count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, nullptr);
		queueFamilies.resize(count);
		vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, queueFamilies.data());
	}

	auto VulkanRenderer::SelectQueueFamily() -> std::optional<int>
	{
		int idx = 0;
		for (auto& prop : queueFamilies)
		{
			if (prop.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
				return idx;
			++idx;
		}
		return {};
	}

	auto VulkanRenderer::CreateDevice(VkPhysicalDevice gpu, uint32_t queueIndex) -> VkResult
	{
		VkResult result;
		float queuePriorities[1] = { 0.0f };

		std::vector<const char*> requestedExtension = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceQueueCreateInfo queueInfo = {};
		queueInfo.queueFamilyIndex = graphicsQueueIndex;
		queueInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.pNext = nullptr;
		queueInfo.queueCount = 1;
		queueInfo.pQueuePriorities = queuePriorities;

		VkDeviceCreateInfo deviceInfo = {};
		deviceInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceInfo.pNext = nullptr;
		deviceInfo.queueCreateInfoCount = 1;
		deviceInfo.pQueueCreateInfos = &queueInfo;
		deviceInfo.enabledLayerCount = 0;
		deviceInfo.ppEnabledLayerNames = nullptr;
		deviceInfo.enabledExtensionCount = requestedExtension.size();
		deviceInfo.ppEnabledExtensionNames = requestedExtension.data();
		deviceInfo.pEnabledFeatures = &deviceFeatures;

		result = vkCreateDevice(gpu, &deviceInfo, nullptr, &device);
		return result;
	}

	auto VulkanRenderer::CreateCommandPool(uint32_t queue) -> VkResult
	{
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.pNext = nullptr;
		poolInfo.queueFamilyIndex = queue;
		poolInfo.flags = VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; //명령 버퍼가 개별적으로 기록되도록 허용

		VkResult result;
		result = vkCreateCommandPool(device, &poolInfo, nullptr, &cmdPool);
		assert(result == VkResult::VK_SUCCESS);

		return result;
	}

	auto VulkanRenderer::ResetCommandPool(uint32_t queue) -> VkResult
	{
		return vkResetCommandPool(device, cmdPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
	}

	bool VulkanRenderer::Init(sh::window::Window& win)
	{
		window = &win;
		winHandle = win.GetNativeHandle();
		//VkResult::Success = 0

		layers.Query();
		if (layers.FindLayer(validationLayerName))
			bFindValidationLayer = true;

		if (CreateInstance()) return false;

		InitDebugMessenger();

		if (GetPhysicalDevices()) return false;

		VkPhysicalDevice gpu;
		if (gpu = SelectPhysicalDevice(); !gpu) return false;
		layers.Query(gpu);

		GetQueueFamilyProperties(gpu);
		//그래픽스 큐의 인덱스 값을 가져온다.
		if (auto idx = SelectQueueFamily(); !idx.has_value()) return false;
		else
			graphicsQueueIndex = *idx;

		if (CreateDevice(gpu, graphicsQueueIndex)) return false;

		if (CreateCommandPool(graphicsQueueIndex)) return false;

		if (surface.CreateSurface(instance)) return false;

		if (sh::core::Util::IsDebug())
		{
			for (auto& i : layers.GetLayerProperties())
			{
				fmt::print("LayerName: {} - {}\n", i.properties.layerName, i.properties.description);
				for (auto& ext : i.extensions)
				{
					fmt::print("ExtensionName: {}\n", ext.extensionName);
				}
			}
			fmt::print("-----GPU Layer------\n");
			for (auto& i : layers.GetGPULayerProperties())
			{
				fmt::print("LayerName: {} - {}\n", i.properties.layerName, i.properties.description);
				for (auto& ext : i.extensions)
				{
					fmt::print("ExtensionName: {}\n", ext.extensionName);
				}
			}
			fmt::print("Vulkan Renderer Init!\n");
		}
		return true;
	}
}//namespace

