#pragma once
#pragma warning(disable: 4251)

#include "../Renderer.h"
#include "VulkanConfig.h"

#include "Core/NonCopyable.h"
#include "Core/ThreadSyncManager.h"

#include "glm/mat4x4.hpp"

#include <string>
#include <vector>
#include <array>
#include <optional>
#include <functional>
#include <memory>
#include <atomic>

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

	class VulkanRenderer :
		public Renderer, public sh::core::INonCopyable{
	public:
		static constexpr int VULKAN_API_VER = VK_API_VERSION_1_1;
	private:
		sh::window::Window* window;

		std::vector<const char*> requestedLayer;
		std::vector<const char*> requestedInstanceExtension;
		std::vector<const char*> requestedDeviceExtension;

		std::unique_ptr<vk::VulkanSwapChain> swapChain;
		std::unique_ptr<VulkanLayer> layers;
		std::unique_ptr<VulkanQueueManager> queueManager;
		core::SyncArray<VkCommandPool> cmdPools;
		core::SyncArray<std::unique_ptr<VulkanCommandBuffer>> cmdBuffer;
		std::unique_ptr<VulkanDescriptorPool> descPool;
		std::unique_ptr<VulkanPipelineManager> pipelineManager;

		VkInstance instance;
		VkPhysicalDeviceProperties gpuProp;
		VkPhysicalDevice gpu;
		VkDevice device; // 논리적 장치
		std::vector<VulkanFramebuffer> framebuffers;

		std::vector<VkPhysicalDevice> gpus;

		VkDebugUtilsMessengerEXT debugMessenger;
		std::string validationLayerName;

		VkSemaphore imageAvailableSemaphore;
		VkSemaphore renderFinishedSemaphore;
		VkSemaphore gameThreadSemaphore;
		VkFence inFlightFence;

		int currentFrame;
		uint32_t descriptorPoolSize;

		VmaAllocator allocator;

		bool isInit : 1;
		bool bFindValidationLayer : 1;
		bool bEnableValidationLayers : 1;
	private:
		auto CreateInstance(const std::vector<const char*>& requestedLayer, const std::vector<const char*>& requestedExtension)->VkResult;
		void DestroyInstance();

		auto CreateDebugInfo()->VkDebugUtilsMessengerCreateInfoEXT;
		void InitDebugMessenger();
		void DestroyDebugMessenger();

		auto GetPhysicalDevices()->VkResult;
		auto SelectPhysicalDevice(const std::function<bool(VkPhysicalDevice)>& checkFunc)->VkPhysicalDevice;
		bool IsDeviceSuitable(VkPhysicalDevice gpu);

		auto CreateDevice(VkPhysicalDevice gpu)->VkResult;
		void DestroyDevice();

		void CreateCommandPool(uint32_t queueFamilyIdx);
		void DestroyCommandPool();

		auto CreateSyncObjects() -> VkResult;
		void DestroySyncObjects();

		void CreateAllocator();
		void DestroyAllocator();

		void PrintLayer();

		void RenderDrawable(IDrawable* drawable, VkPipeline& lastPipeline, VkCommandBuffer cmd);
	public:
		SH_RENDER_API VulkanRenderer(core::ThreadSyncManager& syncManager);
		SH_RENDER_API ~VulkanRenderer();

		SH_RENDER_API bool Init(sh::window::Window& win) override;
		SH_RENDER_API bool Resizing() override;
		SH_RENDER_API void Clean() override;

		SH_RENDER_API bool IsInit() const override;

		SH_RENDER_API void Render(float deltaTime) override;

		SH_RENDER_API void WaitForCurrentFrame();

		SH_RENDER_API void SetViewport(const glm::vec2& start, const glm::vec2& end) override;

		SH_RENDER_API void SurfaceReady();

		SH_RENDER_API auto ResetCommandPools() -> VkResult;

		SH_RENDER_API auto GetInstance() const -> VkInstance;
		SH_RENDER_API auto GetDevice() const -> VkDevice;
		SH_RENDER_API auto GetGPU() const -> VkPhysicalDevice;
		SH_RENDER_API auto GetCommandPool(core::ThreadType thr) const -> VkCommandPool;
		SH_RENDER_API auto GetCommandBuffer(core::ThreadType thr) const -> VulkanCommandBuffer*;
		SH_RENDER_API auto GetQueueManager() const -> VulkanQueueManager&;
		SH_RENDER_API auto GetMainFramebuffer() const -> const Framebuffer* override;
		SH_RENDER_API auto GetDescriptorPool() const -> VulkanDescriptorPool&;
		SH_RENDER_API auto GetCurrentFrame() const -> int;
		SH_RENDER_API auto GetWidth() const -> uint32_t override;
		SH_RENDER_API auto GetHeight() const -> uint32_t override;
		SH_RENDER_API auto GetAllocator() const -> VmaAllocator;
		SH_RENDER_API auto GetGPUProperty() const -> const VkPhysicalDeviceProperties&;
		SH_RENDER_API auto GetRenderFinshedSemaphore() const -> VkSemaphore;
		SH_RENDER_API auto GetGameThreadSemaphore() const -> VkSemaphore;
		SH_RENDER_API auto GetPipelineManager() -> VulkanPipelineManager&;
	};
}//namespace
