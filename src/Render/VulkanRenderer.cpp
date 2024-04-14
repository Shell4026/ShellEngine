#include "VulkanRenderer.h"

#include <fmt/core.h>
#include "../Core/Util.h"
#include "VulkanImpl/VulkanLayer.h"
#include "VulkanImpl/VulkanSurface.h"
#include "VulkanImpl/VulkanPipeline.h"

#include "VulkanShader.h"
#include "ShaderLoader.h"
#include "VulkanShaderBuilder.h"

#include <cassert>
#include <set>


namespace sh::render {
	VulkanRenderer::VulkanRenderer() :
		instance(nullptr), gpu(nullptr), device(nullptr), cmdPool(nullptr), window(nullptr),
		graphicsQueueIndex(-1), surfaceQueueIndex(-1),
		graphicsQueue(nullptr), surfaceQueue(nullptr),
		debugMessenger(nullptr), validationLayerName("VK_LAYER_KHRONOS_validation"),
		isInit(false), bFindValidationLayer(false), bEnableValidationLayers(sh::core::Util::IsDebug())
	{
	}

	VulkanRenderer::~VulkanRenderer()
	{
		if(isInit)
			Clean();
	}

	void VulkanRenderer::Clean()
	{
		DestroyCommandPool();

		pipeline.reset();
		surface.reset();
		DestroyDevice();

		if (bEnableValidationLayers)
			DestroyDebugMessenger();

		DestroyInstance();

		isInit = false;
		fmt::print("Clean VulkanRenderer\n");
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

	auto VulkanRenderer::CreateInstance(const std::vector<const char*>& requestedLayer, const std::vector<const char*>& requestedExtension) -> VkResult
	{


		VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
		if (bFindValidationLayer && bEnableValidationLayers)
			debugInfo = CreateDebugInfo();

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
			instanceInfo.pNext = &debugInfo;
		else
			instanceInfo.pNext = nullptr;
		VkResult result = vkCreateInstance(&instanceInfo, nullptr, &instance);
		return result;
	}

	void VulkanRenderer::DestroyInstance()
	{
		if (instance)
		{
			vkDestroyInstance(instance, nullptr);
			instance = nullptr;
		}
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
		debugInfo.pUserData = nullptr;

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
		assert(gpu);
		if (!gpu)
			return false;

		VkPhysicalDeviceProperties prop;
		vkGetPhysicalDeviceProperties(gpu, &prop);

		VkPhysicalDeviceFeatures feature;
		vkGetPhysicalDeviceFeatures(gpu, &feature);

		bool swapchainExSupport = layers->FindGPUExtension(gpu, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		return 
			((prop.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) ||
			(prop.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_CPU)) && 
			swapchainExSupport && surface->IsSwapChainSupport(gpu);
	}

	auto VulkanRenderer::SelectPhysicalDevice(const std::function<bool(VkPhysicalDevice)>& checkFunc) -> VkPhysicalDevice
	{
		for (auto gpu : gpus)
		{
			layers->Query(gpu);
			if (checkFunc(gpu))
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

	auto VulkanRenderer::SelectQueueFamily(VkQueueFlagBits queueType) -> std::optional<int>
	{
		int idx = 0;
		for (auto& prop : queueFamilies)
		{
			if (prop.queueFlags & queueType)
				return idx;
			++idx;
		}
		return {};
	}

	auto VulkanRenderer::GetSurfaceQueueFamily(VkPhysicalDevice gpu) -> std::optional<int>
	{
		assert(surface->GetSurface() != nullptr);
		assert(gpu != nullptr);
		int idx = 0;
		for (auto& prop : queueFamilies)
		{
			VkBool32 support = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(gpu, idx, surface->GetSurface(), &support);
			if (support)
				return idx;
			++idx;
		}
		return {};
	}

	auto VulkanRenderer::CreateDevice(VkPhysicalDevice gpu) -> VkResult
	{
		assert(gpu);
		VkResult result;

		std::vector<const char*> requestedExtension = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		VkPhysicalDeviceFeatures deviceFeatures{};

		assert(graphicsQueueIndex != -1);
		assert(surfaceQueueIndex != -1);
		std::vector<VkDeviceQueueCreateInfo> queueInfos;
		std::set<uint32_t> queueIdxs = { graphicsQueueIndex, surfaceQueueIndex };

		float queuePriority = 1.0f;
		for (auto idx : queueIdxs)
		{
			VkDeviceQueueCreateInfo queueInfo = {};
			queueInfo.queueFamilyIndex = idx;
			queueInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.pNext = nullptr;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &queuePriority;

			queueInfos.push_back(queueInfo);
		}


		VkDeviceCreateInfo deviceInfo = {};
		deviceInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceInfo.pNext = nullptr;
		deviceInfo.queueCreateInfoCount = queueInfos.size();
		deviceInfo.pQueueCreateInfos = queueInfos.data();
		deviceInfo.enabledLayerCount = 0;
		deviceInfo.ppEnabledLayerNames = nullptr;
		deviceInfo.enabledExtensionCount = requestedExtension.size();
		deviceInfo.ppEnabledExtensionNames = requestedExtension.data();
		deviceInfo.pEnabledFeatures = &deviceFeatures;

		result = vkCreateDevice(gpu, &deviceInfo, nullptr, &device);
		return result;
	}

	void VulkanRenderer::DestroyDevice()
	{
		if (device)
		{
			vkDestroyDevice(device, nullptr);
			device = nullptr;
		}
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

	void VulkanRenderer::DestroyCommandPool()
	{
		if (cmdPool)
		{
			vkDestroyCommandPool(device, cmdPool, nullptr);
			cmdPool = nullptr;
		}
	}

	auto VulkanRenderer::ResetCommandPool(uint32_t queue) -> VkResult
	{
		return vkResetCommandPool(device, cmdPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
	}

	bool VulkanRenderer::Init(sh::window::Window& win)
	{
		window = &win;
		winHandle = win.GetNativeHandle();

		layers = std::make_unique<impl::VulkanLayer>();
		surface = std::make_unique<impl::VulkanSurface>();
		pipeline = std::make_unique<impl::VulkanPipeline>();
		//VkResult::Success = 0

		if (layers->FindVulkanExtension(VK_KHR_SURFACE_EXTENSION_NAME))
			requestedExtension.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
		else
			return false;

		if (bEnableValidationLayers)
		{
			if (layers->FindLayer(validationLayerName))
			{
				bFindValidationLayer = true;
				requestedLayer.push_back(validationLayerName.c_str());
				requestedExtension.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
				requestedExtension.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}
		}

#if _WIN32
		if (layers->FindVulkanExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME))
			requestedExtension.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
		else
			return false;
#elif __linux__
		if (layers->FindVulkanExtension(VK_KHR_XLIB_SURFACE_EXTENSION_NAME))
			requestedExtension.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
		else
			return false;
#endif
		if (CreateInstance(requestedLayer, requestedExtension)) 
			return false;
		
		if (bEnableValidationLayers)
			InitDebugMessenger();

		if (!surface->CreateSurface(win, instance))
			return false;

		if (GetPhysicalDevices())
			return false;

		auto suitableFunc
		{
			[&](VkPhysicalDevice _gpu) -> bool
			{
				return IsDeviceSuitable(_gpu);
			} 
		};
		gpu = SelectPhysicalDevice(suitableFunc);
		if (!gpu) 
			return false;

		GetQueueFamilyProperties(gpu);
		if (auto idx = SelectQueueFamily(VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT); !idx.has_value()) return false;
		else graphicsQueueIndex = *idx;

		if (auto idx = GetSurfaceQueueFamily(gpu); !idx.has_value()) return false;
		else surfaceQueueIndex = *idx;

		if (CreateDevice(gpu)) 
			return false;

		vkGetDeviceQueue(device, graphicsQueueIndex, 0, &graphicsQueue);
		assert(graphicsQueue);
		vkGetDeviceQueue(device, surfaceQueueIndex, 0, &surfaceQueue);
		assert(surfaceQueue);

		surface->CreateSwapChain(device);

		VulkanShaderBuilder shaderBuilder{ device };
		ShaderLoader loader{ &shaderBuilder };
		auto shader = loader.LoadShader<VulkanShader>("vert.spv", "frag.spv");
		assert(shader.get());

		pipeline->CreateRenderPass(surface.get());
		pipeline->CreateGraphicsPipeline(shader.get(), surface.get());
		if (CreateCommandPool(graphicsQueueIndex)) 
			return false;

		isInit = true;
		PrintLayer();

		return true;
	}

	void VulkanRenderer::PrintLayer()
	{
		//if (sh::core::Util::IsDebug())
		{
			for (auto& i : layers->GetLayerProperties())
			{
				fmt::print("LayerName: {} - {}\n", i.properties.layerName, i.properties.description);
				for (auto& ext : i.extensions)
				{
					fmt::print("ExtensionName: {}\n", ext.extensionName);
				}
			}
			fmt::print("-----GPU Layer------\n");
			for (auto& i : layers->GetGPULayerProperties())
			{
				fmt::print("LayerName: {} - {}\n", i.properties.layerName, i.properties.description);
				for (auto& ext : i.extensions)
				{
					fmt::print("ExtensionName: {}\n", ext.extensionName);
				}
			}
			fmt::print("-----Vulkan Extensions-----\n");
			for (auto& i : layers->GetVulkanExtensions())
				fmt::print("ExtensionName: {}\n", i.extensionName);
			fmt::print("-----GPU Extensions------\n");
			for (auto& i : layers->GetGPUExtensions())
				fmt::print("ExtensionName: {}\n", i.extensionName);

			fmt::print("Vulkan Renderer Init!\n");
		}
	}

	bool VulkanRenderer::IsInit() const
	{
		return isInit;
	}
}//namespace

