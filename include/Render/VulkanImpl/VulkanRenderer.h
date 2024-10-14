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

namespace sh::render {
	namespace impl
	{
		class VulkanLayer;
		class VulkanSurface;
		class VulkanPipeline;
		class VulkanCommandBuffer;
		class VulkanFramebuffer;
		class VulkanDescriptorPool;
	}
	class VulkanRenderer :
		public Renderer, public sh::core::INonCopyable{
	public:
		static constexpr int VULKAN_API_VER = VK_API_VERSION_1_1;
	private:
		sh::window::Window* window;
		sh::window::WinHandle winHandle;

		std::vector<const char*> requestedLayer;
		std::vector<const char*> requestedInstanceExtension;
		std::vector<const char*> requestedDeviceExtension;

		std::unique_ptr<impl::VulkanSurface> surface;
		std::unique_ptr<impl::VulkanLayer> layers;
		core::SyncArray<std::unique_ptr<impl::VulkanCommandBuffer>> cmdBuffer;
		std::unique_ptr<impl::VulkanDescriptorPool> descPool;

		VkInstance instance;
		VkPhysicalDeviceProperties gpuProp;
		VkPhysicalDevice gpu;
		VkDevice device; // 논리적 장치
		std::vector<impl::VulkanFramebuffer> framebuffers;

		core::SyncArray<VkCommandPool> cmdPool;

		std::vector<VkPhysicalDevice> gpus;
		std::vector<VkQueueFamilyProperties> queueFamilies;

		VkDebugUtilsMessengerEXT debugMessenger;
		std::string validationLayerName;

		std::pair<uint8_t, uint8_t> graphicsQueueIndex; // 큐패밀리, 큐
		std::pair<uint8_t, uint8_t> surfaceQueueIndex;
		std::pair<uint8_t, uint8_t> transferQueueIndex;
		VkQueue graphicsQueue;
		VkQueue surfaceQueue;
		VkQueue transferQueue; // 큐 하나만 지원하면 nullptr

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

		void GetQueueFamilyProperties(VkPhysicalDevice gpu);
		auto SelectQueueFamily(VkQueueFlagBits queueType) -> std::optional<int>;
		auto GetSurfaceQueueFamily(VkPhysicalDevice gpu)->std::optional<int>;

		auto CreateDevice(VkPhysicalDevice gpu)->VkResult;
		void DestroyDevice();

		void CreateCommandPool(uint32_t queueFamily);
		void DestroyCommandPool();
		auto ResetCommandPool(uint32_t queue) -> VkResult;

		auto CreateSyncObjects() -> VkResult;
		void DestroySyncObjects();

		void CreateAllocator();
		void DestroyAllocator();

		void PrintLayer();
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

		SH_RENDER_API auto GetInstance() const -> VkInstance;
		SH_RENDER_API auto GetDevice() const -> VkDevice;
		SH_RENDER_API auto GetGPU() const -> VkPhysicalDevice;
		SH_RENDER_API auto GetCommandPool(core::ThreadType thr) const -> VkCommandPool;
		SH_RENDER_API auto GetCommandBuffer(core::ThreadType thr) const -> VkCommandBuffer;
		SH_RENDER_API auto GetGraphicsQueue() const -> VkQueue;
		SH_RENDER_API auto GetGraphicsQueueIdx() const -> std::pair<uint8_t, uint8_t>;
		SH_RENDER_API auto GetTransferQueue() const-> VkQueue;
		SH_RENDER_API auto GetTransferQueueIdx() const->std::pair<uint8_t, uint8_t>;
		SH_RENDER_API auto GetMainFramebuffer() const -> const Framebuffer* override;
		SH_RENDER_API auto GetDescriptorPool() const -> impl::VulkanDescriptorPool&;
		SH_RENDER_API auto GetCurrentFrame() const -> int;
		SH_RENDER_API auto GetWidth() const -> uint32_t override;
		SH_RENDER_API auto GetHeight() const -> uint32_t override;
		SH_RENDER_API auto GetAllocator() const -> VmaAllocator;
		SH_RENDER_API auto GetGPUProperty() const -> const VkPhysicalDeviceProperties&;
		SH_RENDER_API auto GetRenderFinshedSemaphore() const -> VkSemaphore;
		SH_RENDER_API auto GetGameThreadSemaphore() const -> VkSemaphore;
	};
}//namespace
