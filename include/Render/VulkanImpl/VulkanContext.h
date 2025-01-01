#pragma once
#include "Export.h"
#include "../IRenderContext.h"
#include "VulkanConfig.h"

#include "Core/ISyncable.h"

#include <vector>
#include <memory>

namespace sh::window
{
	class Window;
}

namespace sh::render::vk
{
	class VulkanLayer;
	class VulkanSwapChain;
	class VulkanPipeline;
	class VulkanCommandBuffer;
	class VulkanFramebuffer;
	class VulkanDescriptorPool;
	class VulkanPipelineManager;
	class VulkanQueueManager;

	class VulkanContext : public IRenderContext
	{
	private:
		static constexpr int VULKAN_API_VER = VK_API_VERSION_1_1;
		static constexpr const char* VALIDATION_LAYER_NAME = "VK_LAYER_KHRONOS_validation";

		const sh::window::Window& window;

		std::vector<const char*> requestedLayer;
		std::vector<const char*> requestedExtension;
		std::vector<const char*> requestedDeviceExtension;
		std::vector<VkPhysicalDevice> gpus;

		std::unique_ptr<VulkanLayer> layers;

		VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
		VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

		VkInstance instance = VK_NULL_HANDLE;

		VkPhysicalDevice gpu = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties gpuProp{};

		VkDevice device = VK_NULL_HANDLE;

		VmaAllocator allocator = VK_NULL_HANDLE;

		std::vector<VulkanFramebuffer> framebuffers;

		std::unique_ptr<VulkanQueueManager> queueManager;
		std::unique_ptr<VulkanSwapChain> swapChain;
		core::SyncArray<VkCommandPool> cmdPools;
		core::SyncArray<std::unique_ptr<VulkanCommandBuffer>> cmdBuffer;
		std::unique_ptr<VulkanDescriptorPool> descPool;
		std::unique_ptr<VulkanPipelineManager> pipelineManager;

		bool bFindValidationLayer = false;
		bool bEnableValidationLayers = false;
	private:
		void PrepareValidationLayer();
		void CreateDebugInfo();
		void PrepareSurfaceExtension();
		void CreateInstance();
		void DestroyInstance();
		void InitDebugMessenger();
		void DestroyDebugMessenger();
		void GetPhysicalDevices();
		auto SelectPhysicalDevice() const -> VkPhysicalDevice;
		void CreateDevice(VkPhysicalDevice gpu);
		void DestroyDevice();
		void CreateAllocator();
		void DestroyAllocator();
		void CreateFrameBuffer();
		void CreateCommandPool(uint32_t queueFamilyIdx);
		void DestroyCommandPool();
		void CreateCommandBuffers();
		void DestroyCommandBuffers();
	public:
		SH_RENDER_API VulkanContext(const sh::window::Window& window);

		SH_RENDER_API void Init() override;
		SH_RENDER_API void Clean() override;
		SH_RENDER_API auto GetRenderAPIType() const -> RenderAPI override;

		SH_RENDER_API auto ReSizing() -> bool;

		SH_RENDER_API void PrintLayers();
		SH_RENDER_API auto ResetCommandPools() -> VkResult;

		SH_RENDER_API auto GetInstance() const -> VkInstance;
		SH_RENDER_API auto GetGPU() const->VkPhysicalDevice;
		SH_RENDER_API auto GetGPUName() const->std::string_view;
		SH_RENDER_API auto GetGPUProperty() const -> const VkPhysicalDeviceProperties&;
		SH_RENDER_API auto GetDevice() const -> VkDevice;
		SH_RENDER_API auto GetSwapChain() const -> VulkanSwapChain&;
		SH_RENDER_API auto GetCommandPool(core::ThreadType thr) const -> VkCommandPool;
		SH_RENDER_API auto GetCommandBuffer(core::ThreadType thr) const -> VulkanCommandBuffer*;
		SH_RENDER_API auto GetQueueManager() const -> VulkanQueueManager&;
		SH_RENDER_API auto GetMainFramebuffer(uint32_t idx = 0) const -> const VulkanFramebuffer*;
		SH_RENDER_API auto GetDescriptorPool() const -> VulkanDescriptorPool&;
		SH_RENDER_API auto GetAllocator() const -> VmaAllocator;
		SH_RENDER_API auto GetPipelineManager() const -> VulkanPipelineManager&;
	};
}