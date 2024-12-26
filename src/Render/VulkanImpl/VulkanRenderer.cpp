#include "pch.h"
#include "VulkanRenderer.h"

#include <fmt/core.h>
#include "../Core/Util.h"

#include "VulkanLayer.h"
#include "VulkanSwapChain.h"
#include "VulkanCommandBuffer.h"
#include "VulkanFramebuffer.h"
#include "VulkanDescriptorPool.h"
#include "VulkanShader.h"
#include "VulkanDrawable.h"
#include "VulkanUniformBuffer.h"
#include "VulkanPipelineManager.h"
#include "VulkanQueueManager.h"

#include "Mesh.h"
#include "Material.h"

#include <cassert>
#include <cstdint>
#include <utility>

#undef min
#undef max

namespace sh::render::vk
{
	SH_RENDER_API VulkanRenderer::VulkanRenderer(core::ThreadSyncManager& syncManager) :
		Renderer(RenderAPI::Vulkan, syncManager),
		instance(nullptr), gpu(nullptr), device(nullptr), window(nullptr),
		debugMessenger(nullptr), validationLayerName("VK_LAYER_KHRONOS_validation"),
		currentFrame(0),
		isInit(false), bFindValidationLayer(false), bEnableValidationLayers(sh::core::Util::IsDebug()),
		allocator(nullptr), 
		descPool(nullptr),
		descriptorPoolSize(10),
		gameThreadSemaphore(nullptr)
	{
	}

	SH_RENDER_API VulkanRenderer::~VulkanRenderer()
	{
		if(isInit)
			Clean();
	}

	SH_RENDER_API void VulkanRenderer::Clean()
	{
		Renderer::Clean();
		if (!device)
			return;

		vkDeviceWaitIdle(device);

		pipelineManager.reset();
		descPool.reset();

		DestroySyncObjects();

		cmdBuffer[core::ThreadType::Game]->Clean();
		cmdBuffer[core::ThreadType::Render]->Clean();

		DestroyCommandPool();

		queueManager.reset();

		framebuffers.clear();
		swapChain.reset();
		DestroyAllocator();
		DestroyDevice();

		if (bEnableValidationLayers)
			DestroyDebugMessenger();

		DestroyInstance();
		
		isInit = false;
		fmt::print("Clean VulkanRenderer\n");
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
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
		appInfo.apiVersion = VULKAN_API_VER;

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
		debugInfo.pfnUserCallback = DebugCallback;
		debugInfo.pUserData = nullptr;

		return debugInfo;
	}

	void VulkanRenderer::InitDebugMessenger()
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func == nullptr)
		{
			bEnableValidationLayers = false;
			return;
		}

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
			swapchainExSupport && swapChain->IsSwapChainSupport(gpu);
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

	auto VulkanRenderer::CreateDevice(VkPhysicalDevice gpu) -> VkResult
	{
		assert(gpu);
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

	void VulkanRenderer::CreateCommandPool(uint32_t queueFamilyIdx)
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
				throw std::runtime_error{ std::string{"vkCreateCommandPool(): "} + string_VkResult(result) };
		}
	}
	void VulkanRenderer::DestroyCommandPool()
	{
		for (auto& cmdPool : cmdPools)
		{
			if (cmdPool == nullptr)
				continue;

			vkDestroyCommandPool(device, cmdPool, nullptr);
			cmdPool = nullptr;
		}
	}

	auto VulkanRenderer::CreateSyncObjects() -> VkResult
	{
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VkFenceCreateFlagBits::VK_FENCE_CREATE_SIGNALED_BIT; //시작부터 신호를 받음

		VkResult result;
		result = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			return result;

		result = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			return result;

		result = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &gameThreadSemaphore);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			return result;

		result = vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			return result;

		return result;
	}

	void VulkanRenderer::DestroySyncObjects()
	{
		vkDestroySemaphore(device, imageAvailableSemaphore, nullptr);
		vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
		vkDestroySemaphore(device, gameThreadSemaphore, nullptr);
		vkDestroyFence(device, inFlightFence, nullptr);
	}

	void VulkanRenderer::CreateAllocator()
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
	}

	void VulkanRenderer::DestroyAllocator()
	{
		if (allocator)
		{
			vmaDestroyAllocator(allocator);
			allocator = nullptr;
		}
	}

	SH_RENDER_API bool VulkanRenderer::Init(sh::window::Window& win)
	{
		Renderer::Init(win);

		window = &win;

		layers = std::make_unique<VulkanLayer>();

		// 표면 확장 탐색
		if (layers->FindVulkanExtension(VK_KHR_SURFACE_EXTENSION_NAME))
			requestedInstanceExtension.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
		else
			return false;

		if (bEnableValidationLayers)
		{
			if (layers->FindLayer(validationLayerName))
			{
				bFindValidationLayer = true;
				requestedLayer.push_back(validationLayerName.c_str());
				requestedInstanceExtension.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
				requestedInstanceExtension.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			}
		}

#if _WIN32
		if (layers->FindVulkanExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME))
			requestedInstanceExtension.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
		else
			return false;
#elif __linux__
		if (layers->FindVulkanExtension(VK_KHR_XLIB_SURFACE_EXTENSION_NAME))
			requestedInstanceExtension.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
		else
			return false;
#endif
		// 인스턴스 생성
		if (CreateInstance(requestedLayer, requestedInstanceExtension) != VkResult::VK_SUCCESS)
			return false;
		
		// 디버그 모드일시 디버그 레이어 생성
		if (bEnableValidationLayers)
			InitDebugMessenger();

		// 표면 생성
		swapChain = std::make_unique<VulkanSwapChain>();
		swapChain->SetContext(instance, nullptr, nullptr);
		swapChain->CreateSurface(win);

		// GPU 목록을 가져온다.
		if (GetPhysicalDevices() != VkResult::VK_SUCCESS)
			return false;

		// 적당한 GPU 선택
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
		vkGetPhysicalDeviceProperties(gpu, &gpuProp);
		fmt::print("GPU: {}\n", gpuProp.deviceName);

		requestedDeviceExtension = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		// 큐 매니저 생성
		queueManager = std::make_unique<VulkanQueueManager>(gpu);

		// 가상 장치 생성
		if (CreateDevice(gpu) != VkResult::VK_SUCCESS)
			return false;

		// 큐 생성
		queueManager->SetDevice(device);
		queueManager->CreateGraphicsQueue();
		queueManager->CreateTransferQueue();
		queueManager->CreateSurfaceQueue(swapChain->GetSurface());

		// 메모리 할당자 생성(VMA라이브러리)
		CreateAllocator();

		// 스왑체인 생성
		swapChain->SetContext(instance, device, gpu);
		swapChain->CreateSwapChain(queueManager->GetGraphicsQueueFamilyIdx(), queueManager->GetSurfaceQueueFamilyIdx(), false);

		// 프레임버퍼 생성 (렌더패스 생성 -> 프레임버퍼 생성)
		auto& imgs = swapChain->GetSwapChainImageViews();
		framebuffers.reserve(imgs.size());
		for (size_t i = 0; i < imgs.size(); ++i)
		{
			framebuffers.push_back(VulkanFramebuffer{ *this });
			VkResult result = framebuffers[i].Create(swapChain->GetSwapChainSize().width, swapChain->GetSwapChainSize().height, imgs[i], swapChain->GetSwapChainImageFormat());
			assert(result == VkResult::VK_SUCCESS);
			if (result != VkResult::VK_SUCCESS)
				return false;
		}

		// 커맨드 풀과 커맨드 버퍼 생성
		CreateCommandPool(queueManager->GetGraphicsQueueFamilyIdx());

		cmdBuffer[core::ThreadType::Game] = std::make_unique<VulkanCommandBuffer>(device, cmdPools[core::ThreadType::Game]);
		cmdBuffer[core::ThreadType::Game]->Create();
		cmdBuffer[core::ThreadType::Render] = std::make_unique<VulkanCommandBuffer>(device, cmdPools[core::ThreadType::Render]);
		cmdBuffer[core::ThreadType::Render]->Create();

		// 세마포어와 펜스 생성 (동기화 변수)
		if (CreateSyncObjects() != VkResult::VK_SUCCESS)
			return false;

		// 디스크립터 풀 생성
		descPool = std::make_unique<VulkanDescriptorPool>(device, 16);

		// 파이프라인 매니저 생성
		pipelineManager = std::make_unique<VulkanPipelineManager>(device);

		isInit = true;
		PrintLayer();

		return true;
	}

	SH_RENDER_API bool VulkanRenderer::Resizing()
	{
		vkDeviceWaitIdle(device);

		DestroySyncObjects();
		CreateSyncObjects();

		framebuffers.clear();
		//swapChain->DestroySwapChain();
		swapChain->CreateSwapChain(queueManager->GetGraphicsQueueFamilyIdx(), queueManager->GetSurfaceQueueFamilyIdx(), false);

		auto& imgs = swapChain->GetSwapChainImageViews();
		framebuffers.reserve(imgs.size());
		for (size_t i = 0; i < imgs.size(); ++i)
		{
			framebuffers.push_back(VulkanFramebuffer{ *this });
			VkResult result = framebuffers[i].Create(swapChain->GetSwapChainSize().width, swapChain->GetSwapChainSize().height, imgs[i], swapChain->GetSwapChainImageFormat());
			assert(result == VkResult::VK_SUCCESS);
			if (result != VkResult::VK_SUCCESS)
				return false;
		}
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
	inline void VulkanRenderer::RenderDrawable(IDrawable* iDrawable, VkPipeline& lastPipeline, VkCommandBuffer cmd)
	{
		if (!core::IsValid(iDrawable))
			return;
		VulkanDrawable* drawable = static_cast<VulkanDrawable*>(iDrawable);
		const Mesh* mesh = drawable->GetMesh();
		const Material* mat = drawable->GetMaterial();

		assert(mesh);
		assert(mat);
		if (!sh::core::IsValid(mesh) || !sh::core::IsValid(mat))
			return;

		VulkanShader* shader = static_cast<VulkanShader*>(mat->GetShader());
		if (!sh::core::IsValid(shader))
			return;

		VulkanPipeline* currentPipeline = drawable->GetPipeline(core::ThreadType::Render);
		if (currentPipeline == nullptr)
			return;
		if (currentPipeline->GetPipeline() == nullptr)
			return;

		if (currentPipeline->GetPipeline() != lastPipeline)
		{
			lastPipeline = currentPipeline->GetPipeline();
			vkCmdBindPipeline(cmd, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, lastPipeline);
		}
		mesh->GetVertexBuffer()->Bind();

		VkDescriptorSet localDescSet = drawable->GetDescriptorSet(core::ThreadType::Render);
		VkDescriptorSet descSet = static_cast<VulkanUniformBuffer*>(mat->GetUniformBuffer(core::ThreadType::Render))->GetVkDescriptorSet();
		std::array<VkDescriptorSet, 2> descriptorSets = { localDescSet, descSet };
		
		assert(localDescSet);
		assert(descSet);

		vkCmdBindDescriptorSets(cmd,
			VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS,
			shader->GetPipelineLayout(), 0, descriptorSets.size(),
			descriptorSets.data(), 0, nullptr);

		vkCmdDrawIndexed(cmd, mesh->GetIndices().size(), 1, 0, 0, 0);
	}

	SH_RENDER_API bool VulkanRenderer::IsInit() const
	{
		return isInit;
	}

	SH_RENDER_API void VulkanRenderer::WaitForCurrentFrame()
	{
		vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
	}

	SH_RENDER_API void VulkanRenderer::Render(float deltaTime)
	{
		if (!isInit || bPause.load(std::memory_order::memory_order_acquire))
			return;
		if (drawList[core::ThreadType::Render].empty())
			return;

		WaitForCurrentFrame();

		uint32_t imgIdx;
		VkResult result = vkAcquireNextImageKHR(device, swapChain->GetSwapChain(), UINT64_MAX, imageAvailableSemaphore, nullptr, &imgIdx);
		if (result == VkResult::VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			if (IsPause())
				return;
			if (result == VkResult::VK_ERROR_OUT_OF_DATE_KHR)
			{
				SH_INFO("Resizing");
				Resizing();
			}
			return;
		}
		if (result == VkResult::VK_ERROR_SURFACE_LOST_KHR)
		{
			return;
		}
		vkResetFences(device, 1, &inFlightFence);

		cmdBuffer[core::ThreadType::Render]->Reset();
		cmdBuffer[core::ThreadType::Render]->SetWaitSemaphore({ imageAvailableSemaphore });
		cmdBuffer[core::ThreadType::Render]->SetSignalSemaphore({ renderFinishedSemaphore });
		cmdBuffer[core::ThreadType::Render]->SetWaitStage({ 
			VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		});

		VkCommandBuffer buffer = cmdBuffer[core::ThreadType::Render]->GetCommandBuffer();
		if (drawList[core::ThreadType::Render].empty())
			return;

		cmdBuffer[core::ThreadType::Render]->Build([&]()
		{
			bool mainPassProcessed = false;

			for (auto& [camera, drawables] : drawList[core::ThreadType::Render])
			{
				auto renderTexture = camera->GetRenderTexture();
				// 프레임버퍼에 그림
				if(renderTexture != nullptr)
				{
					auto framebuffer = static_cast<const VulkanFramebuffer*>(renderTexture->GetFramebuffer(core::ThreadType::Render));
					std::array<VkClearValue, 2> clear;
					clear[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
					clear[1].depthStencil = { 1.0f, 0 };

					VkRenderPassBeginInfo renderPassInfo{};
					renderPassInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
					renderPassInfo.renderPass = framebuffer->GetRenderPass();
					renderPassInfo.framebuffer = framebuffer->GetVkFramebuffer();
					renderPassInfo.renderArea.offset = { 0, 0 };
					renderPassInfo.renderArea.extent = { framebuffer->GetWidth(), framebuffer->GetHeight() };
					renderPassInfo.clearValueCount = static_cast<uint32_t>(clear.size());
					renderPassInfo.pClearValues = clear.data();

					//Begin RenderPass
					vkCmdBeginRenderPass(buffer, &renderPassInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

					VkViewport viewport{};
					viewport.x = 0;
					viewport.y = framebuffer->GetHeight();
					viewport.width = framebuffer->GetWidth();
					viewport.height = -static_cast<float>(framebuffer->GetHeight());
					viewport.minDepth = 0.0f;
					viewport.maxDepth = 1.0f;
					vkCmdSetViewport(buffer, 0, 1, &viewport);

					VkRect2D scissor{};
					scissor.offset = { 0, 0 };
					scissor.extent = { framebuffer->GetWidth(), framebuffer->GetHeight() };
					vkCmdSetScissor(buffer, 0, 1, &scissor);

					VkPipeline lastPipeline = nullptr;
					for (auto iDrawable : drawables)
					{
						RenderDrawable(iDrawable, lastPipeline, buffer);
					}
					//End RenderPass
					vkCmdEndRenderPass(buffer);

					renderTexture->SetDirty();
				}
				//MainPass
				else
				{
					mainPassProcessed = true;
					VkRenderPassBeginInfo renderPassInfo{};
					std::array<VkClearValue, 2> clear;
					clear[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
					clear[1].depthStencil = { 1.0f, 0 };

					renderPassInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
					renderPassInfo.renderPass = framebuffers[imgIdx].GetRenderPass();
					renderPassInfo.framebuffer = framebuffers[imgIdx].GetVkFramebuffer();
					renderPassInfo.renderArea.offset = { 0, 0 };
					renderPassInfo.renderArea.extent = swapChain->GetSwapChainSize();
					renderPassInfo.clearValueCount = static_cast<uint32_t>(clear.size());
					renderPassInfo.pClearValues = clear.data();
					vkCmdBeginRenderPass(buffer, &renderPassInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

					VkViewport viewport{};
					float width = viewportEnd.x - viewportStart.x;
					float height = viewportEnd.y - viewportStart.y;
					float surfWidth = static_cast<float>(swapChain->GetSwapChainSize().width);
					float surfHeight = static_cast<float>(swapChain->GetSwapChainSize().height);
					viewport.x = viewportStart.x;
					viewport.y = viewportEnd.y;
					viewport.width = std::min(width, surfWidth);
					viewport.height = -std::min(height, surfHeight);
					viewport.minDepth = 0.0f;
					viewport.maxDepth = 1.0f;
					vkCmdSetViewport(buffer, 0, 1, &viewport);

					VkRect2D scissor{};
					scissor.offset = { 0, 0 };
					scissor.extent = swapChain->GetSwapChainSize();
					vkCmdSetScissor(buffer, 0, 1, &scissor);

					VkPipeline lastPipeline = nullptr;
					for (auto iDrawable : drawables)
					{
						//RenderDrawable(iDrawable, lastPipeline, buffer);
					}
					for (auto& func : drawCalls)
						func();

					vkCmdEndRenderPass(buffer);
				}
			}//for (auto& [camera, drawables] : drawList[RENDER_THREAD])
			// 모든 카메라에 프레임 버퍼가 존재하는 특수한 경우
			if (mainPassProcessed == false)
			{
				VkRenderPassBeginInfo renderPassInfo{};
				std::array<VkClearValue, 2> clear;
				clear[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
				clear[1].depthStencil = { 1.0f, 0 };

				renderPassInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = framebuffers[imgIdx].GetRenderPass();
				renderPassInfo.framebuffer = framebuffers[imgIdx].GetVkFramebuffer();
				renderPassInfo.renderArea.offset = { 0, 0 };
				renderPassInfo.renderArea.extent = swapChain->GetSwapChainSize();
				renderPassInfo.clearValueCount = static_cast<uint32_t>(clear.size());
				renderPassInfo.pClearValues = clear.data();
				vkCmdBeginRenderPass(buffer, &renderPassInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

				VkViewport viewport{};
				float width = viewportEnd.x - viewportStart.x;
				float height = viewportEnd.y - viewportStart.y;
				float surfWidth = static_cast<float>(swapChain->GetSwapChainSize().width);
				float surfHeight = static_cast<float>(swapChain->GetSwapChainSize().height);
				viewport.x = viewportStart.x;
				viewport.y = viewportEnd.y;
				viewport.width = std::min(width, surfWidth);
				viewport.height = -std::min(height, surfHeight);
				viewport.minDepth = 0.0f;
				viewport.maxDepth = 1.0f;
				vkCmdSetViewport(buffer, 0, 1, &viewport);

				for (auto& func : drawCalls)
					func();

				vkCmdEndRenderPass(buffer);
			}
		}, nullptr);

		queueManager->SubmitCommand(*cmdBuffer[core::ThreadType::Render].get(), inFlightFence);

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
		VkSwapchainKHR swapChains[] = { swapChain->GetSwapChain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imgIdx;
		presentInfo.pResults = nullptr;

		vkQueuePresentKHR(queueManager->GetSurfaceQueue(), &presentInfo);
	}

	void VulkanRenderer::SetViewport(const glm::vec2& start, const glm::vec2& end)
	{
		viewportStart = start;
		viewportEnd = end;
	}

	void VulkanRenderer::SurfaceReady()
	{
		
	}

	SH_RENDER_API auto VulkanRenderer::ResetCommandPools() -> VkResult
	{
		for (auto& cmdPool : cmdPools)
		{
			auto result = vkResetCommandPool(device, cmdPool, VkCommandPoolResetFlagBits::VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
			if (result != VkResult::VK_SUCCESS)
				throw std::runtime_error{ std::string{"Can't reset VkCommandPool!: "} + string_VkResult(result) };
		}
		return VkResult::VK_SUCCESS;
	}

	SH_RENDER_API auto VulkanRenderer::GetInstance() const -> VkInstance
	{
		return instance;
	}
	SH_RENDER_API auto VulkanRenderer::GetDevice() const -> VkDevice
	{
		return device;
	}

	SH_RENDER_API auto VulkanRenderer::GetMainFramebuffer() const -> const Framebuffer*
	{
		return &framebuffers[0];
	}

	SH_RENDER_API auto VulkanRenderer::GetGPU() const -> VkPhysicalDevice
	{
		return gpu;
	}

	SH_RENDER_API auto VulkanRenderer::GetCommandPool(core::ThreadType thr) const -> VkCommandPool
	{
		return cmdPools[thr];
	}
	SH_RENDER_API auto VulkanRenderer::GetCommandBuffer(core::ThreadType thr) const -> VulkanCommandBuffer*
	{
		return cmdBuffer[static_cast<int>(thr)].get();
	}

	SH_RENDER_API auto VulkanRenderer::GetQueueManager() const->VulkanQueueManager&
	{
		return *queueManager.get();
	}
	SH_RENDER_API auto VulkanRenderer::GetDescriptorPool() const -> VulkanDescriptorPool&
	{
		return *descPool.get();
	}
	SH_RENDER_API auto VulkanRenderer::GetCurrentFrame() const -> int
	{
		return currentFrame;
	}
	SH_RENDER_API auto VulkanRenderer::GetWidth() const -> uint32_t
	{
		return swapChain->GetSwapChainSize().width;
	}
	SH_RENDER_API auto VulkanRenderer::GetHeight() const -> uint32_t
	{
		return swapChain->GetSwapChainSize().height;
	}
	SH_RENDER_API auto VulkanRenderer::GetAllocator() const -> VmaAllocator
	{
		return allocator;
	}
	SH_RENDER_API auto VulkanRenderer::GetGPUProperty() const -> const VkPhysicalDeviceProperties&
	{
		return gpuProp;
	}
	SH_RENDER_API auto VulkanRenderer::GetRenderFinshedSemaphore() const -> VkSemaphore
	{
		return renderFinishedSemaphore;
	}
	SH_RENDER_API auto VulkanRenderer::GetGameThreadSemaphore() const -> VkSemaphore
	{
		return gameThreadSemaphore;
	}
	SH_RENDER_API auto VulkanRenderer::GetPipelineManager() -> VulkanPipelineManager&
	{
		return *pipelineManager.get();
	}
}//namespace

