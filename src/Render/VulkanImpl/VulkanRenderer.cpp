#include "pch.h"
#include "VulkanRenderer.h"

#include <fmt/core.h>
#include "../Core/Util.h"

#include "VulkanLayer.h"
#include "VulkanSurface.h"
#include "VulkanCommandBuffer.h"
#include "VulkanFramebuffer.h"
#include "VulkanDescriptorPool.h"
#include "VulkanShader.h"
#include "VulkanDrawable.h"

#include "Mesh.h"
#include "Material.h"

#include <cassert>
#include <cstdint>
#include <utility>

#undef min
#undef max

namespace sh::render {
	VulkanRenderer::VulkanRenderer(core::ThreadSyncManager& syncManager) :
		Renderer(RenderAPI::Vulkan, syncManager),
		instance(nullptr), gpu(nullptr), device(nullptr), window(nullptr),
		graphicsQueueIndex(-1), surfaceQueueIndex(-1),
		graphicsQueue(nullptr), surfaceQueue(nullptr),
		debugMessenger(nullptr), validationLayerName("VK_LAYER_KHRONOS_validation"),
		currentFrame(0),
		isInit(false), bFindValidationLayer(false), bEnableValidationLayers(sh::core::Util::IsDebug()),
		allocator(nullptr), 
		descPool(nullptr),
		descriptorPoolSize(10)
	{
	}

	VulkanRenderer::~VulkanRenderer()
	{
		if(isInit)
			Clean();
	}

	void VulkanRenderer::Clean()
	{
		Renderer::Clean();
		if (!device)
			return;

		vkDeviceWaitIdle(device);

		descPool.reset();

		DestroySyncObjects();

		cmdBuffer[core::ThreadType::Game]->Clean();
		cmdBuffer[core::ThreadType::Render]->Clean();
		DestroyCommandPool();

		framebuffers.clear();
		surface.reset();
		DestroyAllocator();
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
		debugInfo.pfnUserCallback = debugCallback;
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

	void VulkanRenderer::CreateCommandPool(uint32_t queue)
	{
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.pNext = nullptr;
		poolInfo.queueFamilyIndex = queue;
		poolInfo.flags = VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; //명령 버퍼가 개별적으로 기록되도록 허용

		for (int thr = 0; thr < cmdPool.size(); ++thr)
		{
			VkResult result;
			result = vkCreateCommandPool(device, &poolInfo, nullptr, &cmdPool[thr]);
			if (result != VkResult::VK_SUCCESS)
				throw std::runtime_error{ std::string{"Can't create VkCommandPool!: "} + string_VkResult(result) };
		}
	}

	void VulkanRenderer::DestroyCommandPool()
	{
		for (int thr = 0; thr < cmdPool.size(); ++thr)
		{
			if (cmdPool[thr])
			{
				vkDestroyCommandPool(device, cmdPool[thr], nullptr);
				cmdPool[thr] = nullptr;
			}
		}
	}

	auto VulkanRenderer::ResetCommandPool(uint32_t queue) -> VkResult
	{
		for (int thr = 0; thr < cmdPool.size(); ++thr)
		{
			auto result = vkResetCommandPool(device, cmdPool[thr], VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
			if(result != VkResult::VK_SUCCESS)
				throw std::runtime_error{ std::string{"Can't reset VkCommandPool!: "} + string_VkResult(result) };
		}
		return VkResult::VK_SUCCESS;
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

	bool VulkanRenderer::Init(sh::window::Window& win)
	{
		Renderer::Init(win);

		window = &win;
		winHandle = win.GetNativeHandle();

		layers = std::make_unique<impl::VulkanLayer>();
		surface = std::make_unique<impl::VulkanSurface>();

		//표면 확장 탐색
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
		//인스턴스 생성
		if (CreateInstance(requestedLayer, requestedInstanceExtension) != VkResult::VK_SUCCESS)
			return false;
		
		//디버그 모드일시 디버그 레이어 생성
		if (bEnableValidationLayers)
			InitDebugMessenger();

		//표면 생성
		if (!surface->CreateSurface(win, instance))
			return false;

		//GPU 목록을 가져온다.
		if (GetPhysicalDevices() != VkResult::VK_SUCCESS)
			return false;

		//적당한 GPU 선택
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

		//그래픽 큐와 화면 큐 선택
		GetQueueFamilyProperties(gpu);
		if (auto idx = SelectQueueFamily(VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT); !idx.has_value()) return false;
		else graphicsQueueIndex = *idx;
		if (auto idx = GetSurfaceQueueFamily(gpu); !idx.has_value()) return false;
		else surfaceQueueIndex = *idx;

		requestedDeviceExtension = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		//가상 장치 생성
		if (CreateDevice(gpu) != VkResult::VK_SUCCESS)
			return false;
		surface->SetDevice(device);

		//가상 장치에서 큐를 가져온다.
		vkGetDeviceQueue(device, graphicsQueueIndex, 0, &graphicsQueue);
		assert(graphicsQueue);
		vkGetDeviceQueue(device, surfaceQueueIndex, 0, &surfaceQueue);
		assert(surfaceQueue);

		//메모리 할당자 생성(VMA라이브러리)
		CreateAllocator();

		//스왑체인 생성
		surface->CreateSwapChain(gpu, graphicsQueueIndex, surfaceQueueIndex);

		//프레임버퍼 생성 (렌더패스 생성 -> 프레임버퍼 생성)
		auto& imgs = surface->GetSwapChainImageViews();
		framebuffers.reserve(imgs.size());
		for (size_t i = 0; i < imgs.size(); ++i)
		{
			framebuffers.push_back(impl::VulkanFramebuffer{ device, gpu, allocator });
			VkResult result = framebuffers[i].Create(surface->GetSwapChainSize().width, surface->GetSwapChainSize().height, imgs[i], surface->GetSwapChainImageFormat());
			assert(result == VkResult::VK_SUCCESS);
			if (result != VkResult::VK_SUCCESS)
				return false;
		}

		//커맨드 풀과 커맨드 버퍼 생성
		CreateCommandPool(graphicsQueueIndex);

		cmdBuffer[core::ThreadType::Game] = std::make_unique<impl::VulkanCommandBuffer>(device, cmdPool[core::ThreadType::Game]);
		cmdBuffer[core::ThreadType::Game]->Create();
		cmdBuffer[core::ThreadType::Render] = std::make_unique<impl::VulkanCommandBuffer>(device, cmdPool[core::ThreadType::Render]);
		cmdBuffer[core::ThreadType::Render]->Create();
		//세마포어와 펜스 생성 (동기화 변수)
		if (CreateSyncObjects() != VkResult::VK_SUCCESS)
			return false;

		//디스크립터 풀 생성
		descPool = std::make_unique<impl::VulkanDescriptorPool>(device, 16);

		isInit = true;
		PrintLayer();

		return true;
	}

	bool VulkanRenderer::Resizing()
	{
		vkDeviceWaitIdle(device);

		DestroySyncObjects();
		CreateSyncObjects();

		framebuffers.clear();
		surface->DestroySwapChain();

		surface->CreateSwapChain(gpu, graphicsQueueIndex, surfaceQueueIndex);

		auto& imgs = surface->GetSwapChainImageViews();
		framebuffers.reserve(imgs.size());
		for (size_t i = 0; i < imgs.size(); ++i)
		{
			framebuffers.push_back(impl::VulkanFramebuffer{ device, gpu, allocator });
			VkResult result = framebuffers[i].Create(surface->GetSwapChainSize().width, surface->GetSwapChainSize().height, imgs[i], surface->GetSwapChainImageFormat());
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

	bool VulkanRenderer::IsInit() const
	{
		return isInit;
	}

	void VulkanRenderer::WaitForCurrentFrame()
	{
		vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
	}

	void VulkanRenderer::Render(float deltaTime)
	{
		if (!isInit || bPause.load(std::memory_order::memory_order_acquire))
			return;
		if (drawList[core::ThreadType::Render].empty())
			return;
		//std::cout << "Render Start\n";
		//std::cout << "main: " << mainCamera << '\n';
		//mu->lock();

		WaitForCurrentFrame();

		uint32_t imgIdx;
		VkResult result = vkAcquireNextImageKHR(device, surface->GetSwapChain(), UINT64_MAX, imageAvailableSemaphore, nullptr, &imgIdx);
		if (result == VkResult::VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			if (IsPause())
				return;
			std::cout << "resizing\n";
			Resizing();
			return;
		}
		if (result == VkResult::VK_ERROR_SURFACE_LOST_KHR)
		{
			//Resizing();
			return;
		}
		vkResetFences(device, 1, &inFlightFence);

		cmdBuffer[core::ThreadType::Render]->Reset();
		cmdBuffer[core::ThreadType::Render]->SetWaitSemaphore({ imageAvailableSemaphore });
		cmdBuffer[core::ThreadType::Render]->SetSignalSemaphore({ renderFinishedSemaphore });
		cmdBuffer[core::ThreadType::Render]->SetWaitStage({ VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT });

		VkCommandBuffer buffer = cmdBuffer[core::ThreadType::Render]->GetCommandBuffer();
		if (drawList[core::ThreadType::Render].empty())
			return;

		cmdBuffer[core::ThreadType::Render]->Submit(graphicsQueue, [&]()
		{
			bool mainPassProcessed = false;

			for (auto& [camera, drawables] : drawList[core::ThreadType::Render])
			{
				auto renderTexture = camera->GetRenderTexture();
				//프레임버퍼에 그림
				if(renderTexture != nullptr)
				{
					auto framebuffer = static_cast<const impl::VulkanFramebuffer*>(renderTexture->GetFramebuffer());
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
						VulkanDrawable* drawable = static_cast<VulkanDrawable*>(iDrawable);
						Mesh* mesh = drawable->GetMesh();
						Material* mat = drawable->GetMaterial();

						assert(mesh);
						assert(mat);
						if (!sh::core::IsValid(mesh) || !sh::core::IsValid(mat)) 
							continue;

						VulkanShader* shader = static_cast<VulkanShader*>(mat->GetShader());
						if (!sh::core::IsValid(shader)) 
							continue;

						impl::VulkanPipeline* currentPipeline = drawable->GetPipeline(core::ThreadType::Render);
						if (currentPipeline == nullptr)
							continue;
						if (currentPipeline->GetPipeline() == nullptr)
							continue;

						if (currentPipeline->GetPipeline() != lastPipeline)
						{
							lastPipeline = currentPipeline->GetPipeline();
							vkCmdBindPipeline(buffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, lastPipeline);
						}
						mesh->GetVertexBuffer()->Bind();

						VkDescriptorSet descriptorSets[] = { drawable->GetDescriptorSet() };
						vkCmdBindDescriptorSets(buffer,
							VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS,
							shader->GetPipelineLayout(), 0, 1,
							descriptorSets, 0, nullptr);
						vkCmdDrawIndexed(buffer, mesh->GetIndices().size(), 1, 0, 0, 0);
					}
					//End RenderPass
					vkCmdEndRenderPass(buffer);

					//renderTexture->SetDirty();
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
					renderPassInfo.renderArea.extent = surface->GetSwapChainSize();
					renderPassInfo.clearValueCount = static_cast<uint32_t>(clear.size());
					renderPassInfo.pClearValues = clear.data();
					vkCmdBeginRenderPass(buffer, &renderPassInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

					VkViewport viewport{};
					float width = viewportEnd.x - viewportStart.x;
					float height = viewportEnd.y - viewportStart.y;
					float surfWidth = static_cast<float>(surface->GetSwapChainSize().width);
					float surfHeight = static_cast<float>(surface->GetSwapChainSize().height);
					viewport.x = viewportStart.x;
					viewport.y = viewportEnd.y;
					viewport.width = std::min(width, surfWidth);
					viewport.height = -std::min(height, surfHeight);
					viewport.minDepth = 0.0f;
					viewport.maxDepth = 1.0f;
					vkCmdSetViewport(buffer, 0, 1, &viewport);

					VkPipeline lastPipeline = nullptr;
					for (auto iDrawable : drawables)
					{
						VulkanDrawable* drawable = static_cast<VulkanDrawable*>(iDrawable);

						Mesh* mesh = drawable->GetMesh();
						Material* mat = drawable->GetMaterial();

						if (!sh::core::IsValid(mesh) || !sh::core::IsValid(mat)) 
							continue;

						VulkanShader* shader = static_cast<VulkanShader*>(mat->GetShader());
						if (!sh::core::IsValid(shader)) 
							continue;

						impl::VulkanPipeline* currentPipeline = drawable->GetPipeline(core::ThreadType::Render);
						if (currentPipeline == nullptr)
							continue;
						if (currentPipeline->GetPipeline() == nullptr)
							continue;

						if (currentPipeline->GetPipeline() != lastPipeline)
						{
							lastPipeline = currentPipeline->GetPipeline();
							vkCmdBindPipeline(buffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, lastPipeline);
						}

						VkRect2D scissor{};
						scissor.offset = { 0, 0 };
						scissor.extent = surface->GetSwapChainSize();
						vkCmdSetScissor(buffer, 0, 1, &scissor);

						mesh->GetVertexBuffer()->Bind();

						VkDescriptorSet descriptorSets[] = { drawable->GetDescriptorSet() };
						vkCmdBindDescriptorSets(buffer,
							VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS,
							shader->GetPipelineLayout(), 0, 1,
							descriptorSets, 0, nullptr);
						vkCmdDrawIndexed(buffer, mesh->GetIndices().size(), 1, 0, 0, 0);
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
				renderPassInfo.renderArea.extent = surface->GetSwapChainSize();
				renderPassInfo.clearValueCount = static_cast<uint32_t>(clear.size());
				renderPassInfo.pClearValues = clear.data();
				vkCmdBeginRenderPass(buffer, &renderPassInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

				VkViewport viewport{};
				float width = viewportEnd.x - viewportStart.x;
				float height = viewportEnd.y - viewportStart.y;
				float surfWidth = static_cast<float>(surface->GetSwapChainSize().width);
				float surfHeight = static_cast<float>(surface->GetSwapChainSize().height);
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
		}, nullptr, inFlightFence); //submitEnd

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderFinishedSemaphore;
		VkSwapchainKHR swapChains[] = { surface->GetSwapChain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imgIdx;
		presentInfo.pResults = nullptr;

		vkQueuePresentKHR(surfaceQueue, &presentInfo);
	}

	void VulkanRenderer::SetViewport(const glm::vec2& start, const glm::vec2& end)
	{
		viewportStart = start;
		viewportEnd = end;
	}

	void VulkanRenderer::SurfaceReady()
	{
		
	}

	auto VulkanRenderer::GetInstance() const -> VkInstance
	{
		return instance;
	}
	auto VulkanRenderer::GetDevice() const -> VkDevice
	{
		return device;
	}

	auto VulkanRenderer::GetMainFramebuffer() const -> const Framebuffer*
	{
		return &framebuffers[0];
	}

	auto VulkanRenderer::GetGPU() const -> VkPhysicalDevice
	{
		return gpu;
	}

	auto VulkanRenderer::GetCommandPool(core::ThreadType thr) const -> VkCommandPool
	{
		return cmdPool[thr];
	}
	auto VulkanRenderer::GetCommandBuffer(core::ThreadType thr) const -> VkCommandBuffer
	{
		return cmdBuffer[static_cast<int>(thr)]->GetCommandBuffer();
	}

	auto VulkanRenderer::GetGraphicsQueue() const -> VkQueue
	{
		return graphicsQueue;
	}
	auto VulkanRenderer::GetGraphicsQueueIdx() const -> uint32_t
	{
		return graphicsQueueIndex;
	}

	auto VulkanRenderer::GetDescriptorPool() const -> impl::VulkanDescriptorPool&
	{
		return *descPool.get();
	}
	auto VulkanRenderer::GetCurrentFrame() const -> int
	{
		return currentFrame;
	}
	auto VulkanRenderer::GetWidth() const -> uint32_t
	{
		return surface->GetSwapChainSize().width;
	}
	auto VulkanRenderer::GetHeight() const -> uint32_t
	{
		return surface->GetSwapChainSize().height;
	}
	auto VulkanRenderer::GetAllocator() const -> VmaAllocator
	{
		return allocator;
	}
	auto VulkanRenderer::GetGPUProperty() const -> const VkPhysicalDeviceProperties&
	{
		return gpuProp;
	}
}//namespace

