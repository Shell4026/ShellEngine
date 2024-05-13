#include "VulkanRenderer.h"

#include <fmt/core.h>
#include "../Core/Util.h"
#include "VulkanImpl/VulkanLayer.h"
#include "VulkanImpl/VulkanSurface.h"
#include "VulkanImpl/VulkanPipeline.h"
#include "VulkanImpl/VulkanCommandBuffer.h"
#include "VulkanImpl/VulkanFramebuffer.h"

#include "Mesh.h"
#include "Material.h"
#include "VulkanShader.h"
#include "ShaderLoader.h"
#include "VulkanShaderBuilder.h"
#include "VulkanDrawable.h"

#include <cassert>
#include <set>
#include <cstdint>
#include <limits>
#include <exception>

namespace sh::render {
	VulkanRenderer::VulkanRenderer() :
		instance(nullptr), gpu(nullptr), device(nullptr), cmdPool(nullptr), window(nullptr),
		graphicsQueueIndex(-1), surfaceQueueIndex(-1),
		graphicsQueue(nullptr), surfaceQueue(nullptr),
		debugMessenger(nullptr), validationLayerName("VK_LAYER_KHRONOS_validation"),
		currentFrame(0),
		isInit(false), bPause(false), bFindValidationLayer(false), bEnableValidationLayers(sh::core::Util::IsDebug()),
		Renderer(RenderAPI::Vulkan)
	{
	}

	VulkanRenderer::~VulkanRenderer()
	{
		if(isInit)
			Clean();
	}

	void VulkanRenderer::Clean()
	{
		if (!device)
			return;

		vkDeviceWaitIdle(device);

		DestroySyncObjects();

		for(auto& buffer : cmdBuffers)
			buffer->Reset();
		DestroyCommandPool();

		framebuffers.clear();
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

	auto VulkanRenderer::CreateSyncObjects() -> VkResult
	{
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VkFenceCreateFlagBits::VK_FENCE_CREATE_SIGNALED_BIT; //시작부터 신호를 받음

		VkResult result;
		for (int i = 0; i < MAX_FRAME_DRAW; ++i)
		{
			result = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore[i]);
			assert(result == VkResult::VK_SUCCESS);
			if (result != VkResult::VK_SUCCESS)
				return result;

			result = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore[i]);
			assert(result == VkResult::VK_SUCCESS);
			if (result != VkResult::VK_SUCCESS)
				return result;

			result = vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence[i]);
			assert(result == VkResult::VK_SUCCESS);
			if (result != VkResult::VK_SUCCESS)
				return result;
		}
		return result;
	}

	void VulkanRenderer::DestroySyncObjects()
	{
		for (int i = 0; i < MAX_FRAME_DRAW; ++i)
		{
			vkDestroySemaphore(device, imageAvailableSemaphore[i], nullptr);
			vkDestroySemaphore(device, renderFinishedSemaphore[i], nullptr);
			vkDestroyFence(device, inFlightFence[i], nullptr);
		}
	}

	bool VulkanRenderer::Init(sh::window::Window& win)
	{
		window = &win;
		winHandle = win.GetNativeHandle();

		layers = std::make_unique<impl::VulkanLayer>();
		surface = std::make_unique<impl::VulkanSurface>();

		//표면 확장 탐색
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
		//인스턴스 생성
		if (CreateInstance(requestedLayer, requestedExtension) != VkResult::VK_SUCCESS)
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

		//그래픽 큐와 화면 큐 선택
		GetQueueFamilyProperties(gpu);
		if (auto idx = SelectQueueFamily(VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT); !idx.has_value()) return false;
		else graphicsQueueIndex = *idx;
		if (auto idx = GetSurfaceQueueFamily(gpu); !idx.has_value()) return false;
		else surfaceQueueIndex = *idx;

		//가상 장치 생성
		if (CreateDevice(gpu) != VkResult::VK_SUCCESS)
			return false;
		surface->SetDevice(device);

		//가상 장치에서 큐를 가져온다.
		vkGetDeviceQueue(device, graphicsQueueIndex, 0, &graphicsQueue);
		assert(graphicsQueue);
		vkGetDeviceQueue(device, surfaceQueueIndex, 0, &surfaceQueue);
		assert(surfaceQueue);


		//스왑체인 생성
		surface->CreateSwapChain(gpu, graphicsQueueIndex, surfaceQueueIndex);

		//프레임버퍼 생성 (렌더패스 생성 -> 프레임버퍼 생성)
		auto& imgs = surface->GetSwapChainImageViews();
		framebuffers.resize(imgs.size(), impl::VulkanFramebuffer{ device });
		for (int i = 0; i < imgs.size(); ++i)
		{
			VkResult result = framebuffers[i].Create(surface->GetSwapChainSize().width, surface->GetSwapChainSize().height, imgs[i], surface->GetSwapChainImageFormat());
			assert(result == VkResult::VK_SUCCESS);
			if (result != VkResult::VK_SUCCESS)
				return false;
		}

		//커맨드 풀과 커맨드 버퍼 생성
		if (CreateCommandPool(graphicsQueueIndex)) 
			return false;

		for (auto& cmdBuffer : cmdBuffers)
		{
			cmdBuffer = std::make_unique<impl::VulkanCommandBuffer>(device, cmdPool);
			cmdBuffer->Create();
		}
		//세마포어와 펜스 생성 (동기화 변수)
		if (CreateSyncObjects() != VkResult::VK_SUCCESS)
			return false;

		isInit = true;
		PrintLayer();

		return true;
	}

	bool VulkanRenderer::Resizing()
	{
		framebuffers.clear();
		surface->DestroySwapChain();

		surface->CreateSwapChain(gpu, graphicsQueueIndex, surfaceQueueIndex);

		auto& imgs = surface->GetSwapChainImageViews();
		framebuffers.resize(imgs.size(), impl::VulkanFramebuffer{ device });
		for (int i = 0; i < imgs.size(); ++i)
		{
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

	void VulkanRenderer::Render(float deltaTime)
	{
		if (!isInit || bPause)
			return;

		vkWaitForFences(device, 1, &inFlightFence[currentFrame], VK_TRUE, UINT64_MAX);

		uint32_t imgIdx;
		VkResult result = vkAcquireNextImageKHR(device, surface->GetSwapChain(), UINT64_MAX, imageAvailableSemaphore[currentFrame], nullptr, &imgIdx);
		if (result == VkResult::VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			Resizing();
			return;
		}
		vkResetFences(device, 1, &inFlightFence[currentFrame]);

		cmdBuffers[currentFrame]->Reset();
		cmdBuffers[currentFrame]->SetWaitSemaphore({ imageAvailableSemaphore[currentFrame] });
		cmdBuffers[currentFrame]->SetSignalSemaphore({ renderFinishedSemaphore[currentFrame] });
		cmdBuffers[currentFrame]->SetWaitStage({ VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT });

		VkCommandBuffer buffer = cmdBuffers[currentFrame]->GetCommandBuffer();

		cmdBuffers[currentFrame]->Submit(graphicsQueue, [&]()
		{
				VkRenderPassBeginInfo renderPassInfo{};
				VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
				renderPassInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = framebuffers[imgIdx].GetRenderPass();
				renderPassInfo.framebuffer = framebuffers[imgIdx].GetVkFramebuffer();
				renderPassInfo.renderArea.offset = { 0, 0 };
				renderPassInfo.renderArea.extent = surface->GetSwapChainSize();
				renderPassInfo.clearValueCount = 1;
				renderPassInfo.pClearValues = &clearColor;

				vkCmdBeginRenderPass(buffer, &renderPassInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);
				while (!drawList.empty())
				{
					Mesh* drawObj = drawList.front();
					drawList.pop();

					sh::render::Material* mat = drawObj->GetMaterial(0);
					if (!sh::core::IsValid(mat)) continue;

					VulkanShader* shader = static_cast<VulkanShader*>(mat->GetShader());
					if (!sh::core::IsValid(shader)) continue;

					VulkanDrawable* drawable = static_cast<VulkanDrawable*>(drawObj->GetDrawable());

					vkCmdBindPipeline(buffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, drawable->GetPipeline()->GetPipeline());

					VkViewport viewport{};
					viewport.x = 0.0f;
					viewport.y = 0.0f;
					viewport.width = static_cast<float>(surface->GetSwapChainSize().width);
					viewport.height = static_cast<float>(surface->GetSwapChainSize().height);
					viewport.minDepth = 0.0f;
					viewport.maxDepth = 1.0f;
					vkCmdSetViewport(buffer, 0, 1, &viewport);

					VkRect2D scissor{};
					scissor.offset = { 0, 0 };
					scissor.extent = surface->GetSwapChainSize();
					vkCmdSetScissor(buffer, 0, 1, &scissor);

					VkBuffer vertexBuffers[] = { drawable->GetVertexBuffer().GetBuffer()};
					VkDeviceSize offsets[] = { 0 };
					vkCmdBindVertexBuffers(buffer, 0, 1, vertexBuffers, offsets);
					vkCmdDraw(buffer, drawObj->GetVertexCount(), 1, 0, 0);
				}
				vkCmdEndRenderPass(buffer);
			},
			inFlightFence[currentFrame]
		);

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &renderFinishedSemaphore[currentFrame];
		VkSwapchainKHR swapChains[] = { surface->GetSwapChain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imgIdx;
		presentInfo.pResults = nullptr;

		vkQueuePresentKHR(surfaceQueue, &presentInfo);

		currentFrame = (currentFrame + 1) % MAX_FRAME_DRAW;
	}

	void VulkanRenderer::Pause(bool b)
	{
		bPause = b;
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
}//namespace

