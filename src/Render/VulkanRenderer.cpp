#include "VulkanRenderer.h"

#include <fmt/core.h>
#include "../Core/Util.h"

namespace sh::render {
	VulkanRenderer::VulkanRenderer() :
		instance(nullptr), device(nullptr), surface(nullptr),
		graphicsQueueIndex(0)
	{
	}

	VulkanRenderer::~VulkanRenderer()
	{
		if (!device)
			vkDestroyDevice(device, nullptr);
		if (!surface)
			vkDestroySurfaceKHR(instance, surface, nullptr);
		if (!instance)
			vkDestroyInstance(instance, nullptr);
	}

	auto VulkanRenderer::GetInstanceLayerProperties() -> VkResult
	{
		std::vector<VkLayerProperties> layerProperties;
		uint32_t instanceLayerCount = 0;

		VkResult result;
		do
		{
			result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);

			layerProperties.resize(instanceLayerCount);
			result = vkEnumerateInstanceLayerProperties(&instanceLayerCount, layerProperties.data());
		} while (result == VkResult::VK_INCOMPLETE);

		for (auto& prop : layerProperties)
		{
			LayerProperties layerProp;
			layerProp.properties = prop;

			result = GetLayerExtensions(layerProp);
			if (result != VkResult::VK_SUCCESS)
				continue;

			layers.push_back(std::move(layerProp));
		}
		return result;
	}

	auto VulkanRenderer::GetLayerExtensions(LayerProperties& layerProp, VkPhysicalDevice gpu) -> VkResult
	{
		uint32_t extensionCount = 0;
		VkResult result;

		char* layerName = layerProp.properties.layerName;
		do
		{
			if (gpu)
				result = vkEnumerateDeviceExtensionProperties(gpu, layerName, &extensionCount, nullptr);
			else
				result = vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, nullptr);

			if (result != VkResult::VK_SUCCESS || extensionCount == 0)
				continue;

			layerProp.extensions.resize(extensionCount);
			if(gpu)
				result = vkEnumerateDeviceExtensionProperties(gpu, layerName, &extensionCount, layerProp.extensions.data());
			else
				result = vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, layerProp.extensions.data());
		} while (result == VkResult::VK_INCOMPLETE);

		return result;
	}

	auto VulkanRenderer::CreateInstance() -> VkResult
	{
		std::vector<const char*> requestedLayer;
		//requestedLayer.push_back("VK_LAYER_LUNARG_api_dump");

		std::vector<const char*> requestedExtension = { VK_KHR_SURFACE_EXTENSION_NAME };
#if _WIN32
		requestedExtension.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif __linux__
		requestedExtension.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif
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
		instanceInfo.pNext = nullptr;
		instanceInfo.flags = 0; // 현재 사용되지 않음
		instanceInfo.pApplicationInfo = &appInfo;
		instanceInfo.enabledLayerCount = requestedLayer.size();
		instanceInfo.ppEnabledLayerNames = requestedLayer.data();
		instanceInfo.enabledExtensionCount = requestedExtension.size();
		instanceInfo.ppEnabledExtensionNames = requestedExtension.data();
		//pAllocator = 호스트 메모리의 할당 방법 지정
		VkResult result = vkCreateInstance(&instanceInfo, nullptr, &instance);
		return result;
	}

	auto VulkanRenderer::CreateSurface() -> VkResult
	{
#if _WIN32
		VkWin32SurfaceCreateInfoKHR createInfo{};
		createInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = winHandle;
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
		return VkResult::VK_ERROR_NOT_PERMITTED_KHR;
	}

	auto VulkanRenderer::GetPhysicalDevices() -> VkResult
	{
		unsigned int count;
		vkEnumeratePhysicalDevices(instance, &count, nullptr);
		gpus.resize(count);

		return vkEnumeratePhysicalDevices(instance, &count, gpus.data());
	}

	auto VulkanRenderer::GetPhysicalDeviceExtensions(VkPhysicalDevice gpu) -> VkResult
	{
		VkResult result = VkResult::VK_SUCCESS;

		for (auto& layer : layers)
		{
			LayerProperties layerProp;
			layerProp.properties = layer.properties;
			result = GetLayerExtensions(layerProp, gpu);

			GPULayers.push_back(layerProp);
		}
		return result;
	}

	bool VulkanRenderer::IsDeviceSuitable(VkPhysicalDevice gpu)
	{
		if (!gpu)
			return false;

		VkPhysicalDeviceProperties prop;
		vkGetPhysicalDeviceProperties(gpu, &prop);

		VkPhysicalDeviceFeatures feature;
		vkGetPhysicalDeviceFeatures(gpu, &feature);

		return prop.deviceType == 
			VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || 
			VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_CPU;
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
		queueFamilyProps.resize(count);
		vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, queueFamilyProps.data());
	}

	auto VulkanRenderer::SelectQueueFamily() -> int
	{
		int idx = 0;
		for (auto& prop : queueFamilyProps)
		{
			if (prop.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
				return idx;
			++idx;
		}
		return -1;
	}

	auto VulkanRenderer::CreateDevice(VkPhysicalDevice gpu, uint32_t queueIndex) -> VkResult
	{
		VkResult result;
		float queuePriorities[1] = { 0.0f };

		std::vector<const char*> requestedExtension = { };

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
		deviceInfo.pEnabledFeatures = nullptr;

		result = vkCreateDevice(gpu, &deviceInfo, nullptr, &device);
		return result;
	}

	auto VulkanRenderer::CreateCommandPool() -> VkResult
	{
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.pNext = nullptr;
		return VkResult();
	}

	bool VulkanRenderer::Init(sh::window::Window& win)
	{
		window = &win;
		winHandle = win.GetNativeHandle();

		if (GetInstanceLayerProperties())
			return false;
		if (CreateInstance())
			return false;
		if (CreateSurface())
			return false;

		if (GetPhysicalDevices())
			return false;
		auto gpu = SelectPhysicalDevice();
		if (!gpu)
			return false;

		if (GetPhysicalDeviceExtensions(gpu))
			return false;

		GetQueueFamilyProperties(gpu);
		//그래픽스 큐의 인덱스 값을 가져온다.
		if (int idx = SelectQueueFamily(); idx == -1)
			return false;
		else
			graphicsQueueIndex = idx;

		if (CreateDevice(gpu, graphicsQueueIndex))
			return false;

		if (sh::core::Util::IsDebug())
		{
			for (auto& i : layers)
			{
				fmt::print("LayerName: {} - {}\n", i.properties.layerName, i.properties.description);
				for (auto& ext : i.extensions)
				{
					fmt::print("ExtensionName: {}\n", ext.extensionName);
				}
			}
			fmt::print("-----GPU Layer------\n");
			for (auto& i : GPULayers)
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

