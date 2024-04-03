#include "VulkanRenderer.h"

#include <fmt/core.h>
#include "../Core/Util.h"

namespace sh::render {
	VulkanRenderer::VulkanRenderer() :
		instance(nullptr)
	{
	}

	VulkanRenderer::~VulkanRenderer()
	{
		if(!instance)
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
			result = GetLayerExtensions(layer, gpu);
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

		return prop.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
	}

	auto VulkanRenderer::CreateInstance() -> VkResult
	{
		std::vector<const char*> requestedLayer = { "VK_LAYER_LUNARG_api_dump" };
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

	auto VulkanRenderer::CreateDevice() -> VkResult
	{
		VkResult result;
		float queuePriorities[1] = { 0.0f };

		VkDeviceQueueCreateInfo queueInfo = {};
		queueInfo.queueFamilyIndex = graphicsQueueIndex;
		queueInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.pNext = nullptr;
		queueInfo.queueCount = 1;
		queueInfo.pQueuePriorities = queuePriorities;

		return VkResult();
	}

	bool VulkanRenderer::Init()
	{
		GetInstanceLayerProperties();
		CreateInstance();
		GetPhysicalDevices();
		for(auto gpu : gpus)
			GetPhysicalDeviceExtensions(gpu);

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
		}
		return true;
	}
}//namespace

