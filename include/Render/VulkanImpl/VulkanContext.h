#pragma once
#include "Export.h"
#include "../IRenderContext.h"
#include "VulkanConfig.h"

#include "Core/ISyncable.h"

#include <glm/vec2.hpp>

#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <shared_mutex>
namespace sh::window
{
	class Window;
}

namespace sh::render::vk
{
	class VulkanLayer;
	class VulkanSwapChain;
	class VulkanPipeline;
	class VulkanCommandBufferPool;
	class VulkanCommandBuffer;
	class VulkanFramebuffer;
	class VulkanDescriptorPool;
	class VulkanPipelineManager;
	class VulkanQueueManager;
	class VulkanRenderPass;
	class VulkanRenderPassManager;

	class VulkanContext : public IRenderContext
	{
	public:
		SH_RENDER_API VulkanContext(const sh::window::Window& window);
		SH_RENDER_API ~VulkanContext();

		SH_RENDER_API void Init() override;
		SH_RENDER_API void Clean() override;
		SH_RENDER_API auto GetRenderAPIType() const -> RenderAPI override;

		SH_RENDER_API auto ReSizing() -> bool;

		SH_RENDER_API void PrintLayers();

		SH_RENDER_API auto FindSupportedDepthFormat(bool bUseStencil) const -> VkFormat;

		/// @brief 멀티 샘플링의 샘플을 지정한다. 기기에서 지원하지 않는다면 최대 지원하는 샘플 수로 지정된다.
		/// @param sample 샘플 수
		SH_RENDER_API void SetSampleCount(VkSampleCountFlagBits sample);
		SH_RENDER_API auto GetSampleCount() const -> VkSampleCountFlagBits;

		SH_RENDER_API auto GetInstance() const -> VkInstance;
		SH_RENDER_API auto GetGPU() const -> VkPhysicalDevice;
		SH_RENDER_API auto GetGPUName() const->std::string_view;
		SH_RENDER_API auto GetGPUProperty() const -> const VkPhysicalDeviceProperties&;
		SH_RENDER_API auto GetMaxSampleCount() const ->VkSampleCountFlagBits;
		SH_RENDER_API auto GetDevice() const -> VkDevice;
		SH_RENDER_API auto GetSwapChain() const -> VulkanSwapChain&;
		SH_RENDER_API auto GetCommandBufferPool() const -> VulkanCommandBufferPool&;
		SH_RENDER_API auto GetQueueManager() const -> VulkanQueueManager&;
		SH_RENDER_API auto GetMainRenderPass() const -> VulkanRenderPass&;
		SH_RENDER_API auto GetUIRenderPass() const -> VulkanRenderPass&;
		SH_RENDER_API auto GetMainFramebuffer(uint32_t idx = 0) const -> const VulkanFramebuffer*;
		SH_RENDER_API auto GetDescriptorPool() const -> VulkanDescriptorPool&;
		SH_RENDER_API auto GetAllocator() const -> VmaAllocator;
		SH_RENDER_API auto GetPipelineManager() const -> VulkanPipelineManager&;
		SH_RENDER_API auto GetRenderPassManager() const -> VulkanRenderPassManager&;
		SH_RENDER_API auto GetEmptyDescriptorSetLayout() const -> VkDescriptorSetLayout;
		SH_RENDER_API auto GetEmptyDescriptorSet() const->VkDescriptorSet;

		SH_RENDER_API void SetViewport(const glm::vec2& start, const glm::vec2& end) override;
		SH_RENDER_API auto GetViewportStart() const -> const glm::vec2& override;
		SH_RENDER_API auto GetViewportEnd() const -> const glm::vec2& override;
	private:
		void PrepareValidationLayer();
		void CreateDebugInfo();
		void PrepareSurfaceExtension();
		void CreateInstance();
		void DestroyInstance();
		void InitDebugMessenger();
		void DestroyDebugMessenger();
		void GetPhysicalDevices();
		auto SelectPhysicalDevice() const->VkPhysicalDevice;
		void CreateDevice(VkPhysicalDevice gpu);
		void DestroyDevice();
		void CreateAllocator();
		void DestroyAllocator();
		void CreateRenderPass();
		void CreateFrameBuffer();
		void CreateCommandPool();
		void DestroyCommandPool();
		void CreateEmptyDescriptor();
	private:
		static constexpr int VULKAN_API_VER = VK_API_VERSION_1_2;
		static constexpr const char* VALIDATION_LAYER_NAME = "VK_LAYER_KHRONOS_validation";

		const sh::window::Window& window;

		std::vector<const char*> requestedLayer;
		std::vector<const char*> requestedExtension;
		std::vector<const char*> requestedDeviceExtension;
		std::vector<VkPhysicalDevice> gpus;

		std::unique_ptr<VulkanLayer> layers;

		VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
		std::vector<VkValidationFeatureEnableEXT> validationEnables;
		VkValidationFeaturesEXT validationFeatures{};
		VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

		VkInstance instance = VK_NULL_HANDLE;

		VkPhysicalDevice gpu = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties gpuProp{};

		VkDevice device = VK_NULL_HANDLE;

		VmaAllocator allocator = VK_NULL_HANDLE;

		VulkanRenderPass* mainRenderPass = nullptr;
		VulkanRenderPass* uiRenderPass = nullptr;
		std::vector<VulkanFramebuffer> framebuffers;

		std::unique_ptr<VulkanQueueManager> queueManager;
		std::unique_ptr<VulkanSwapChain> swapChain;
		std::unique_ptr<VulkanCommandBufferPool> cmdPool;
		std::unique_ptr<VulkanDescriptorPool> descPool;
		std::unique_ptr<VulkanPipelineManager> pipelineManager;
		std::unique_ptr<VulkanRenderPassManager> renderPassManager;

		VkDescriptorSetLayout emptyDescLayout;
		VkDescriptorSet emptyDescSet;

		glm::vec2 viewportStart;
		glm::vec2 viewportEnd;

		VkSampleCountFlagBits sample;

		mutable std::shared_mutex commandPoolMutex;
		mutable std::shared_mutex commandBufferMutex;

		bool bInit = false;
		bool bFindValidationLayer = false;
		bool bEnableValidationLayers = false;
	};
}