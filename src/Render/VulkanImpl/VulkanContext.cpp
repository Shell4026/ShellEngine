#include "VulkanContext.h"
#include "VulkanLayer.h"
#include "VulkanSwapChain.h"
#include "VulkanQueueManager.h"
#include "VulkanFramebuffer.h"
#include "VulkanCommandBuffer.h"
#include "VulkanDescriptorPool.h"
#include "VulkanPipelineManager.h"
#include "VulkanRenderPass.h"
#include "VulkanRenderPassManager.h"

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
			requestedExtension.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
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
		SH_INFO_FORMAT("{}", pCallbackData->pMessage);
		return VK_FALSE;
	}
	void VulkanContext::CreateDebugInfo()
	{
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
		instanceInfo.pNext = bEnableValidationLayers ? &debugInfo : nullptr;
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

		const float defaultPriority{ 0.f };

		uint8_t graphicsIdx = queueManager->GetGraphicsQueueFamilyIdx();
		uint8_t transferIdx = queueManager->GetTransferQueueFamilyIdx();
		uint8_t surfaceIdx = queueManager->GetSurfaceQueueFamilyIdx();

		std::vector<VkDeviceQueueCreateInfo> queueInfos;

		{
			VkDeviceQueueCreateInfo queueInfo = {};
			queueInfo.queueFamilyIndex = graphicsIdx;
			queueInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.pNext = nullptr;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultPriority;
			queueInfos.push_back(queueInfo);
		}
		if (transferIdx != graphicsIdx)
		{
			VkDeviceQueueCreateInfo queueInfo = {};
			queueInfo.queueFamilyIndex = transferIdx;
			queueInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.pNext = nullptr;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultPriority;
			queueInfos.push_back(queueInfo);
		}
		if (surfaceIdx != graphicsIdx && surfaceIdx != transferIdx)
		{
			VkDeviceQueueCreateInfo queueInfo = {};
			queueInfo.queueFamilyIndex = surfaceIdx;
			queueInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueInfo.pNext = nullptr;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &defaultPriority;
			queueInfos.push_back(queueInfo);
		}

		VkDeviceCreateInfo deviceInfo = {};
		deviceInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceInfo.pNext = nullptr;
		deviceInfo.queueCreateInfoCount = queueInfos.size();
		deviceInfo.pQueueCreateInfos = queueInfos.data();
		deviceInfo.enabledLayerCount = 0;
		deviceInfo.ppEnabledLayerNames = nullptr;
		deviceInfo.enabledExtensionCount = requestedDeviceExtension.size();
		deviceInfo.ppEnabledExtensionNames = requestedDeviceExtension.data();
		deviceInfo.pEnabledFeatures = &deviceFeatures;

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
	void VulkanContext::CreateRenderPass()
	{
		renderPassManager = std::make_unique<VulkanRenderPassManager>(*this);
		VulkanRenderPass::Config config{};
		config.format = swapChain->GetSwapChainImageFormat();
		config.depthFormat = FindSupportedDepthFormat(true);
		config.bOffScreen = false;
		config.bTransferSrc = false;
		config.bUseStencil = true;

		mainRenderPass = &renderPassManager->GetOrCreateRenderPass(config);
	}
	void VulkanContext::CreateFrameBuffer()
	{
		framebuffers.clear();

		auto& imgViews = swapChain->GetSwapChainImageViews();
		framebuffers.reserve(imgViews.size());
		for (size_t i = 0; i < imgViews.size(); ++i)
		{
			framebuffers.push_back(VulkanFramebuffer{ *this });

			VkResult result = framebuffers[i].Create(*mainRenderPass, swapChain->GetSwapChainSize().width, swapChain->GetSwapChainSize().height, imgViews[i], swapChain->GetSwapChainImageFormat());
			assert(result == VkResult::VK_SUCCESS);
			if (result != VkResult::VK_SUCCESS)
				throw std::runtime_error(std::string{ "Can't create framebuffer: " } + string_VkResult(result));
		}
	}
	void VulkanContext::CreateCommandPool(uint32_t queueFamilyIdx)
	{
		assert(device != nullptr);

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.pNext = nullptr;
		poolInfo.queueFamilyIndex = queueFamilyIdx;
		poolInfo.flags = VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; //명령 버퍼가 개별적으로 기록되도록 허용

		for (auto& cmdPool : cmdPools)
		{
			VkResult result;
			result = vkCreateCommandPool(device, &poolInfo, nullptr, &cmdPool);
			if (result != VkResult::VK_SUCCESS)
				throw std::runtime_error{ std::string{ "Can't create VkCommandPool: " } + string_VkResult(result) };
		}
	}
	void VulkanContext::DestroyCommandPool()
	{
		for (auto& cmdPool : cmdPools)
		{
			if (cmdPool == nullptr)
				continue;

			vkDestroyCommandPool(device, cmdPool, nullptr);
			cmdPool = nullptr;
		}
	}
	void VulkanContext::CreateCommandBuffers()
	{
		VkResult result;
		cmdBuffer[core::ThreadType::Game] = std::make_unique<VulkanCommandBuffer>(device, cmdPools[core::ThreadType::Game]);
		result = cmdBuffer[core::ThreadType::Game]->Create();
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			throw std::runtime_error(std::string{ "Can't create VkCommandBuffer: " } + string_VkResult(result));

		cmdBuffer[core::ThreadType::Render] = std::make_unique<VulkanCommandBuffer>(device, cmdPools[core::ThreadType::Render]);
		result = cmdBuffer[core::ThreadType::Render]->Create();
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			throw std::runtime_error(std::string{ "Can't create VkCommandBuffer: " } + string_VkResult(result));
	}
	void VulkanContext::DestroyCommandBuffers()
	{
		cmdBuffer[core::ThreadType::Game]->Clear();
		cmdBuffer[core::ThreadType::Render]->Clear();
	}
	void VulkanContext::CreateEmptyDescriptor()
	{
		VkDescriptorSetLayoutCreateInfo info{};
		info.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.bindingCount = 0;

		vkCreateDescriptorSetLayout(device, &info, nullptr, &emptyDescLayout);

		emptyDescSet = descPool->AllocateDescriptorSet(emptyDescLayout, 1);
	}
	VulkanContext::VulkanContext(const sh::window::Window& window) :
		window(window)
	{
		bEnableValidationLayers = core::Util::IsDebug();

		requestedDeviceExtension = 
		{ 
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		};
	}
	SH_RENDER_API void VulkanContext::Init()
	{
		layers = std::make_unique<VulkanLayer>();
		layers->Query();

		if (bEnableValidationLayers)
			PrepareValidationLayer();

		PrepareSurfaceExtension();

		CreateInstance();

		if (bEnableValidationLayers)
			InitDebugMessenger();

		swapChain = std::make_unique<VulkanSwapChain>();
		swapChain->SetContext(instance, nullptr, nullptr);
		swapChain->CreateSurface(window);

		GetPhysicalDevices();

		gpu = SelectPhysicalDevice();
		assert(gpu != nullptr);
		if (gpu == nullptr)
			throw std::runtime_error("Can't find suitable GPU");
		vkGetPhysicalDeviceProperties(gpu, &gpuProp);

		queueManager = std::make_unique<VulkanQueueManager>(gpu);

		CreateDevice(gpu);

		queueManager->SetDevice(device);
		queueManager->CreateGraphicsQueue();
		queueManager->CreateTransferQueue();
		queueManager->CreateSurfaceQueue(swapChain->GetSurface());

		CreateAllocator();

		swapChain->SetContext(instance, device, gpu);
		swapChain->CreateSwapChain(queueManager->GetGraphicsQueueFamilyIdx(), queueManager->GetSurfaceQueueFamilyIdx(), false);

		CreateRenderPass();
		CreateFrameBuffer();

		CreateCommandPool(queueManager->GetGraphicsQueueFamilyIdx());
		CreateCommandBuffers();

		descPool = std::make_unique<VulkanDescriptorPool>(device);
		CreateEmptyDescriptor();

		pipelineManager = std::make_unique<VulkanPipelineManager>(*this);
	}
	SH_RENDER_API void VulkanContext::Clean()
	{
		vkDeviceWaitIdle(device);

		pipelineManager.reset();
		if (emptyDescLayout)
		{
			vkDestroyDescriptorSetLayout(device, emptyDescLayout, nullptr);
			emptyDescLayout = nullptr;
		}
		descPool.reset();
		DestroyCommandBuffers();
		DestroyCommandPool();
		framebuffers.clear();
		renderPassManager.reset();
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
	SH_RENDER_API auto VulkanContext::GetRenderAPIType() const -> RenderAPI
	{
		return RenderAPI::Vulkan;
	}
	SH_RENDER_API auto VulkanContext::ReSizing() -> bool
	{
		framebuffers.clear();
		//swapChain->DestroySwapChain();
		swapChain->CreateSwapChain(queueManager->GetGraphicsQueueFamilyIdx(), queueManager->GetSurfaceQueueFamilyIdx(), false);

		auto& imgs = swapChain->GetSwapChainImageViews();
		framebuffers.reserve(imgs.size());

		CreateFrameBuffer();
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
	SH_RENDER_API auto VulkanContext::ResetCommandPools() -> VkResult
	{
		for (auto& cmdPool : cmdPools)
		{
			auto result = vkResetCommandPool(device, cmdPool, VkCommandPoolResetFlagBits::VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
			if (result != VkResult::VK_SUCCESS)
				throw std::runtime_error{ std::string{"Can't reset VkCommandPool!: "} + string_VkResult(result) };
		}
		return VkResult::VK_SUCCESS;
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
	SH_RENDER_API auto sh::render::vk::VulkanContext::GetGPUName() const -> std::string_view
	{
		return gpuProp.deviceName;
	}
	SH_RENDER_API auto VulkanContext::GetGPUProperty() const -> const VkPhysicalDeviceProperties&
	{
		return gpuProp;
	}
	SH_RENDER_API auto sh::render::vk::VulkanContext::GetInstance() const -> VkInstance
	{
		return instance;
	}
	SH_RENDER_API auto sh::render::vk::VulkanContext::GetGPU() const -> VkPhysicalDevice
	{
		return gpu;
	}
	SH_RENDER_API auto sh::render::vk::VulkanContext::GetDevice() const -> VkDevice
	{
		return device;
	}
	SH_RENDER_API auto VulkanContext::GetSwapChain() const -> VulkanSwapChain&
	{
		return *swapChain.get();
	}
	SH_RENDER_API auto sh::render::vk::VulkanContext::GetCommandPool(core::ThreadType thr) const -> VkCommandPool
	{
		return cmdPools[thr];
	}
	SH_RENDER_API auto sh::render::vk::VulkanContext::GetCommandBuffer(core::ThreadType thr) const -> VulkanCommandBuffer*
	{
		return cmdBuffer[thr].get();
	}
	SH_RENDER_API auto sh::render::vk::VulkanContext::GetQueueManager() const -> VulkanQueueManager&
	{
		return *queueManager.get();
	}
	SH_RENDER_API auto VulkanContext::GetMainRenderPass() const -> VkRenderPass
	{
		return mainRenderPass->GetVkRenderPass();
	}
	SH_RENDER_API auto sh::render::vk::VulkanContext::GetMainFramebuffer(uint32_t idx) const -> const VulkanFramebuffer*
	{
		if (idx >= framebuffers.size())
			return nullptr;
		return &framebuffers[idx];
	}
	SH_RENDER_API auto sh::render::vk::VulkanContext::GetDescriptorPool() const -> VulkanDescriptorPool&
	{
		return *descPool.get();
	}
	SH_RENDER_API auto sh::render::vk::VulkanContext::GetAllocator() const -> VmaAllocator
	{
		return allocator;
	}
	SH_RENDER_API auto sh::render::vk::VulkanContext::GetPipelineManager() const -> VulkanPipelineManager&
	{
		return *pipelineManager.get();
	}
	SH_RENDER_API auto VulkanContext::GetRenderPassManager() const -> VulkanRenderPassManager&
	{
		return *renderPassManager.get();
	}
	SH_RENDER_API auto VulkanContext::GetEmptyDescriptorSetLayout() const -> VkDescriptorSetLayout
	{
		return emptyDescLayout;
	}
	SH_RENDER_API auto VulkanContext::GetEmptyDescriptorSet() const -> VkDescriptorSet
	{
		return emptyDescSet;
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
}//namespace