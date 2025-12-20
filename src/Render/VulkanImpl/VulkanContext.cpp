#include "VulkanContext.h"
#include "VulkanLayer.h"
#include "VulkanSwapChain.h"
#include "VulkanQueueManager.h"
#include "VulkanCommandBufferPool.h"
#include "VulkanCommandBuffer.h"
#include "VulkanDescriptorPool.h"
#include "VulkanPipelineManager.h"
#include "VulkanRenderImpl.h"

#include "Core/Util.h"
#include "Core/Logger.h"

#include <stdexcept>
#include <string>
#include <string_view>
namespace sh::render::vk
{
	void VulkanContext::PrepareValidationLayer()
	{
		if (layers->FindLayer(VALIDATION_LAYER_NAME))
		{
			bFindValidationLayer = true;
			requestedLayer.push_back(VALIDATION_LAYER_NAME);
			requestedExtension.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

			CreateDebugInfo();
		}
	}
	VKAPI_ATTR auto VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
		VkDebugUtilsMessageTypeFlagsEXT messageType, 
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
		void* pUserData) -> VkBool32
	{
		if (messageSeverity == VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
			SH_ERROR_FORMAT("{}", pCallbackData->pMessage);
		else if (messageSeverity == VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			SH_WARN_FORMAT("{}", pCallbackData->pMessage);
		else
			SH_INFO_FORMAT("{}", pCallbackData->pMessage);
		return VK_FALSE;
	}
	void VulkanContext::CreateDebugInfo()
	{
		//validationEnables.push_back(VkValidationFeatureEnableEXT::VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT);
		validationEnables.push_back(VkValidationFeatureEnableEXT::VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT);
		//validationEnables.push_back(VkValidationFeatureEnableEXT::VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT);

		validationFeatures.sType = VkStructureType::VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
		validationFeatures.enabledValidationFeatureCount = validationEnables.size();
		validationFeatures.pEnabledValidationFeatures = validationEnables.data();
		validationFeatures.pNext = &debugInfo;

		debugInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugInfo.messageSeverity =
			VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugInfo.messageType =
			VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugInfo.pfnUserCallback = DebugCallback;
		debugInfo.pUserData = nullptr;
	}
	void VulkanContext::PrepareSurfaceExtension()
	{
		if (layers->FindExtension(VK_KHR_SURFACE_EXTENSION_NAME))
			requestedExtension.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
		else
			throw std::runtime_error(std::string{ "Can't found Vulkan extension: " } + VK_KHR_SURFACE_EXTENSION_NAME);
#if _WIN32
		if (layers->FindExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME))
			requestedExtension.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
		else
			throw std::runtime_error(std::string{ "Can't found Vulkan extension: " } + VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif __linux__
		if (layers->FindExtension(VK_KHR_XLIB_SURFACE_EXTENSION_NAME))
			requestedExtension.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
		else
			throw std::runtime_error(std::string{ "Can't found Vulkan extension: " } + VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif
	}
	void VulkanContext::CreateInstance()
	{
		VkApplicationInfo appInfo = {};
		appInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = nullptr;
		appInfo.pApplicationName = "ShellEngine";
		appInfo.applicationVersion = 1;
		appInfo.pEngineName = "ShellEngine";
		appInfo.engineVersion = 1;
		appInfo.apiVersion = VULKAN_API_VER;

		VkInstanceCreateInfo instanceInfo = {};
		instanceInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceInfo.flags = 0; // 현재 사용되지 않음
		instanceInfo.pApplicationInfo = &appInfo;
		instanceInfo.enabledLayerCount = requestedLayer.size();
		instanceInfo.ppEnabledLayerNames = requestedLayer.data();
		instanceInfo.enabledExtensionCount = requestedExtension.size();
		instanceInfo.ppEnabledExtensionNames = requestedExtension.data();
		instanceInfo.pNext = bEnableValidationLayers ? &validationFeatures : nullptr;
		VkResult result = vkCreateInstance(&instanceInfo, nullptr, &instance);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			throw std::runtime_error("Can't create Vulkan instance!");
	}
	void VulkanContext::DestroyInstance()
	{
		if (instance != VK_NULL_HANDLE)
		{
			vkDestroyInstance(instance, nullptr);
			instance = VK_NULL_HANDLE;
		}
	}
	void VulkanContext::InitDebugMessenger()
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func == nullptr)
		{
			bEnableValidationLayers = false;
			return;
		}

		VkResult result = func(instance, &debugInfo, nullptr, &debugMessenger);
		assert(result == VkResult::VK_SUCCESS);
	}
	void VulkanContext::DestroyDebugMessenger()
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		assert(func != nullptr);
		func(instance, debugMessenger, nullptr);
	}
	void VulkanContext::GetPhysicalDevices()
	{
		uint32_t count;

		VkResult result;

		result = vkEnumeratePhysicalDevices(instance, &count, nullptr);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			throw std::runtime_error("Failed vkEnumeratePhysicalDevices()");
		gpus.resize(count);

		result = vkEnumeratePhysicalDevices(instance, &count, gpus.data());
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			throw std::runtime_error("Failed vkEnumeratePhysicalDevices()");
	}
	auto VulkanContext::SelectPhysicalDevice() const -> VkPhysicalDevice
	{
		for (auto gpu : gpus)
		{
			assert(gpu != nullptr);
			layers->Query(gpu);

			VkPhysicalDeviceProperties prop;
			vkGetPhysicalDeviceProperties(gpu, &prop);

			VkPhysicalDeviceFeatures feature;
			vkGetPhysicalDeviceFeatures(gpu, &feature);

			bool swapchainExSupport = layers->FindGPUExtension(gpu, VK_KHR_SWAPCHAIN_EXTENSION_NAME);

			if (prop.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || 
				prop.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_CPU && 
				swapchainExSupport && 
				swapChain->IsSwapChainSupport(gpu))
				return gpu;
		}
		return nullptr;
	}
	void VulkanContext::CreateDevice(VkPhysicalDevice gpu)
	{
		assert(gpu != nullptr);
		VkResult result;

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = true;

		std::vector<VkDeviceQueueCreateInfo> queueInfos = queueManager->BuildQueueCreateInfos();

		VkPhysicalDeviceTimelineSemaphoreFeatures timelineFeatures{};
		timelineFeatures.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
		timelineFeatures.timelineSemaphore = VK_TRUE;
		timelineFeatures.pNext = nullptr;

		VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicFeatures{};
		dynamicFeatures.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
		dynamicFeatures.dynamicRendering = true;
		dynamicFeatures.pNext = &timelineFeatures;

		VkDeviceCreateInfo deviceInfo = {};
		deviceInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceInfo.queueCreateInfoCount = queueInfos.size();
		deviceInfo.pQueueCreateInfos = queueInfos.data();
		deviceInfo.enabledLayerCount = 0;
		deviceInfo.ppEnabledLayerNames = nullptr;
		deviceInfo.enabledExtensionCount = requestedDeviceExtension.size();
		deviceInfo.ppEnabledExtensionNames = requestedDeviceExtension.data();
		deviceInfo.pEnabledFeatures = &deviceFeatures;
		deviceInfo.pNext = &dynamicFeatures;

		result = vkCreateDevice(gpu, &deviceInfo, nullptr, &device);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			throw std::runtime_error(std::string{ "Can't create vkDevice: " } + string_VkResult(result));
	}
	void VulkanContext::DestroyDevice()
	{
		if (device != VK_NULL_HANDLE)
		{
			vkDestroyDevice(device, nullptr);
			device = VK_NULL_HANDLE;
		}
	}
	void VulkanContext::CreateAllocator()
	{
		VmaAllocatorCreateInfo info{};
		info.instance = instance;
		info.device = device;
		info.physicalDevice = gpu;
		info.flags = VmaAllocatorCreateFlagBits::VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
		info.preferredLargeHeapBlockSize = 0;
		info.vulkanApiVersion = VULKAN_API_VER;
		auto result = vmaCreateAllocator(&info, &allocator);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			throw std::runtime_error(std::string{ "Can't create VMA allocator: " } + string_VkResult(result));
	}
	void VulkanContext::DestroyAllocator()
	{
		if (allocator == VK_NULL_HANDLE)
			return;

		vmaDestroyAllocator(allocator);
	}
	void VulkanContext::CreateCommandPool()
	{
		cmdPool = std::make_unique<VulkanCommandBufferPool>(
			*this, 
			queueManager->GetFamilyIndex(VulkanQueueManager::Role::Graphics), 
			queueManager->GetFamilyIndex(VulkanQueueManager::Role::Transfer));
	}
	void VulkanContext::DestroyCommandPool()
	{
		cmdPool.reset();
	}
	void VulkanContext::CreateEmptyDescriptor()
	{
		VkDescriptorSetLayoutCreateInfo info{};
		info.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.bindingCount = 0;

		vkCreateDescriptorSetLayout(device, &info, nullptr, &emptyDescLayout);

		emptyDescSet = descPool->AllocateDescriptorSet(emptyDescLayout);
	}
	VulkanContext::VulkanContext(const sh::window::Window& window) :
		window(window),
		sample(VkSampleCountFlagBits::VK_SAMPLE_COUNT_4_BIT)
	{
		bEnableValidationLayers = core::Util::IsDebug();

		requestedDeviceExtension = 
		{ 
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
		};
	}
	VulkanContext::~VulkanContext()
	{

	}

	SH_RENDER_API void VulkanContext::Init()
	{
		if (bInit)
			return;
		bInit = true;

		layers = std::make_unique<VulkanLayer>();
		layers->Query();

		if (bEnableValidationLayers)
			PrepareValidationLayer();

		PrepareSurfaceExtension();

		CreateInstance();

		if (bEnableValidationLayers)
			InitDebugMessenger();

		swapChain = std::make_unique<VulkanSwapChain>(*this);
		swapChain->CreateSurface(window);

		GetPhysicalDevices();

		gpu = SelectPhysicalDevice();
		assert(gpu != nullptr);
		if (gpu == nullptr)
			throw std::runtime_error("Can't find suitable GPU");
		vkGetPhysicalDeviceProperties(gpu, &gpuProp);

		queueManager = std::make_unique<VulkanQueueManager>(*this);
		queueManager->QueryFamilies(swapChain->GetSurface());

		CreateDevice(gpu);
		
		queueManager->FetchQueues();

		CreateAllocator();

		swapChain->CreateSwapChain(
			queueManager->GetFamilyIndex(VulkanQueueManager::Role::Graphics), 
			queueManager->GetFamilyIndex(VulkanQueueManager::Role::Present), 
			false);

		CreateCommandPool();

		descPool = std::make_unique<VulkanDescriptorPool>(device);
		CreateEmptyDescriptor();

		pipelineManager = std::make_unique<VulkanPipelineManager>(*this);

		renderImpl = std::make_unique<VulkanRenderImpl>(*this);
	}
	SH_RENDER_API void VulkanContext::Clear()
	{
		bInit = false;

		vkDeviceWaitIdle(device);

		pipelineManager.reset();
		if (emptyDescLayout)
		{
			vkDestroyDescriptorSetLayout(device, emptyDescLayout, nullptr);
			emptyDescLayout = nullptr;
		}
		descPool.reset();
		DestroyCommandPool();
		swapChain.reset();
		queueManager.reset();
		DestroyAllocator();
		DestroyDevice();
		if (bEnableValidationLayers)
			DestroyDebugMessenger();
		gpu = VK_NULL_HANDLE;
		gpus.clear();
		DestroyInstance();
	}
	SH_RENDER_API auto VulkanContext::AllocateCommandBuffer() -> CommandBuffer*
	{
		auto cmd = GetCommandBufferPool().AllocateCommandBuffer(std::this_thread::get_id(), VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT);
		return cmd;
	}
	SH_RENDER_API void VulkanContext::DeallocateCommandBuffer(CommandBuffer& cmd)
	{
		GetCommandBufferPool().DeallocateCommandBuffer(static_cast<VulkanCommandBuffer&>(cmd));
	}
	SH_RENDER_API void VulkanContext::SetViewport(const glm::vec2& start, const glm::vec2& end)
	{
		viewportStart = start;
		viewportEnd = end;
	}

	SH_RENDER_API auto VulkanContext::GetViewportStart() const -> const glm::vec2&
	{
		return viewportStart;
	}
	SH_RENDER_API auto VulkanContext::GetViewportEnd() const -> const glm::vec2&
	{
		return viewportEnd;
	}

	SH_RENDER_API auto VulkanContext::GetRenderImpl() const -> IRenderImpl&
	{
		return *renderImpl;
	}

	SH_RENDER_API auto VulkanContext::ReSizing() -> bool
	{
		//swapChain->DestroySwapChain();
		swapChain->CreateSwapChain(
			queueManager->GetFamilyIndex(VulkanQueueManager::Role::Graphics), 
			queueManager->GetFamilyIndex(VulkanQueueManager::Role::Present), 
			false);
		return true;
	}
	SH_RENDER_API void VulkanContext::PrintLayers()
	{
		for (auto& prop : layers->GetLayerProperties())
		{
			SH_INFO_FORMAT("LayerName: {} - {}", prop.properties.layerName, prop.properties.description);
			for (auto& ext : prop.extensions)
				SH_INFO_FORMAT("ExtensionName: {}", ext.extensionName);
		}
		SH_INFO("-----Vulkan Extensions-----");
		for (auto& i : layers->GetVulkanExtensions())
			SH_INFO_FORMAT("ExtensionName: {}", i.extensionName);

		SH_INFO("-----GPU Layer------");
		for (auto& i : layers->GetGPULayerProperties())
		{
			SH_INFO_FORMAT("LayerName: {} - {}", i.properties.layerName, i.properties.description);
			for (auto& ext : i.extensions)
				SH_INFO_FORMAT("ExtensionName: {}", ext.extensionName);
		}
		SH_INFO("-----GPU Extensions------");
		for (auto& i : layers->GetGPUExtensions())
			SH_INFO_FORMAT("ExtensionName: {}", i.extensionName);
	}
	SH_RENDER_API auto VulkanContext::FindSupportedDepthFormat(bool bUseStencil) const -> VkFormat
	{
		const std::array<VkFormat, 3> formats = { VkFormat::VK_FORMAT_D32_SFLOAT_S8_UINT, VkFormat::VK_FORMAT_D24_UNORM_S8_UINT, VkFormat::VK_FORMAT_D16_UNORM_S8_UINT };

		auto feature = VkFormatFeatureFlagBits::VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
		for (VkFormat format : formats)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(gpu, format, &props);

			if (props.optimalTilingFeatures & feature)
				return format;
		}

		throw std::runtime_error("Failed to find supported Depth format!");
	}
	SH_RENDER_API void VulkanContext::SetSampleCount(VkSampleCountFlagBits sample)
	{
		VkSampleCountFlagBits maxSample = GetMaxSampleCount();
		if (sample > maxSample)
			sample = maxSample;

		this->sample = sample;
	}
	SH_RENDER_API auto VulkanContext::GetMaxSampleCount() const -> VkSampleCountFlagBits
	{
		VkSampleCountFlags supportedSampleCount = 
			std::min(gpuProp.limits.framebufferColorSampleCounts, gpuProp.limits.framebufferDepthSampleCounts);

		if (supportedSampleCount & VkSampleCountFlagBits::VK_SAMPLE_COUNT_64_BIT) return VkSampleCountFlagBits::VK_SAMPLE_COUNT_64_BIT;
		if (supportedSampleCount & VkSampleCountFlagBits::VK_SAMPLE_COUNT_32_BIT) return VkSampleCountFlagBits::VK_SAMPLE_COUNT_32_BIT;
		if (supportedSampleCount & VkSampleCountFlagBits::VK_SAMPLE_COUNT_16_BIT) return VkSampleCountFlagBits::VK_SAMPLE_COUNT_16_BIT;
		if (supportedSampleCount & VkSampleCountFlagBits::VK_SAMPLE_COUNT_8_BIT) return VkSampleCountFlagBits::VK_SAMPLE_COUNT_8_BIT;
		if (supportedSampleCount & VkSampleCountFlagBits::VK_SAMPLE_COUNT_4_BIT) return VkSampleCountFlagBits::VK_SAMPLE_COUNT_4_BIT;
		if (supportedSampleCount & VkSampleCountFlagBits::VK_SAMPLE_COUNT_2_BIT) return VkSampleCountFlagBits::VK_SAMPLE_COUNT_2_BIT;
		
		return VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
	}
}//namespace