#pragma once
#include "../Export.h"
#include "../IRenderContext.h"
#include "VulkanConfig.h"

#include "Core/ISyncable.h"

#include <glm/vec2.hpp>

#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <shared_mutex>
#include <queue>
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
	class VulkanDescriptorPool;
	class VulkanPipelineManager;
	class VulkanQueueManager;
	class VulkanRenderImpl;

	class VulkanContext : public IRenderContext
	{
	public:
		SH_RENDER_API VulkanContext(const sh::window::Window& window);
		SH_RENDER_API ~VulkanContext();

		SH_RENDER_API void Init() override;
		SH_RENDER_API void Clear() override;

		SH_RENDER_API auto GetRenderAPIType() const -> RenderAPI override { return RenderAPI::Vulkan; }

		SH_RENDER_API auto AllocateCommandBuffer() -> CommandBuffer* override;
		SH_RENDER_API void DeallocateCommandBuffer(CommandBuffer& cmd) override;

		SH_RENDER_API void SetViewport(const glm::vec2& start, const glm::vec2& end) override;
		SH_RENDER_API auto GetViewportStart() const -> const glm::vec2 & override;
		SH_RENDER_API auto GetViewportEnd() const -> const glm::vec2 & override;

		SH_RENDER_API auto GetRenderImpl() const -> IRenderImpl& override;

		SH_RENDER_API auto ReSizing() -> bool;

		SH_RENDER_API void PrintLayers();

		SH_RENDER_API auto FindSupportedDepthFormat(bool bUseStencil) const -> VkFormat;

		/// @brief 멀티 샘플링의 샘플을 지정한다. 기기에서 지원하지 않는다면 최대 지원하는 샘플 수로 지정된다.
		/// @param sample 샘플 수
		SH_RENDER_API void SetSampleCount(VkSampleCountFlagBits sample);
		SH_RENDER_API auto GetSampleCount() const -> VkSampleCountFlagBits { return sample; }

		SH_RENDER_API auto GetInstance() const -> VkInstance { return instance; }
		SH_RENDER_API auto GetGPU() const -> VkPhysicalDevice { return gpu; }
		SH_RENDER_API auto GetGPUName() const -> std::string_view { return gpuProp.deviceName; }
		SH_RENDER_API auto GetGPUProperty() const -> const VkPhysicalDeviceProperties& { return gpuProp; }
		SH_RENDER_API auto GetDevice() const -> VkDevice { return device; }
		SH_RENDER_API auto GetSwapChain() const -> VulkanSwapChain& { return *swapChain; }
		SH_RENDER_API auto GetCommandBufferPool() const -> VulkanCommandBufferPool& { return *cmdPool; }
		SH_RENDER_API auto GetQueueManager() const -> VulkanQueueManager& { return *queueManager; }
		SH_RENDER_API auto GetDescriptorPool() const -> VulkanDescriptorPool& { return *descPool; }
		SH_RENDER_API auto GetAllocator() const -> VmaAllocator { return allocator; }
		SH_RENDER_API auto GetPipelineManager() const -> VulkanPipelineManager& { return *pipelineManager; }
		SH_RENDER_API auto GetEmptyDescriptorSetLayout() const -> VkDescriptorSetLayout { return emptyDescLayout; }
		SH_RENDER_API auto GetEmptyDescriptorSet() const -> VkDescriptorSet { return emptyDescSet; }
		SH_RENDER_API auto GetMaxSampleCount() const ->VkSampleCountFlagBits;
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

		std::unique_ptr<VulkanQueueManager> queueManager;
		std::unique_ptr<VulkanSwapChain> swapChain;
		std::unique_ptr<VulkanCommandBufferPool> cmdPool;
		std::unique_ptr<VulkanDescriptorPool> descPool;
		std::unique_ptr<VulkanPipelineManager> pipelineManager;
		std::unique_ptr<VulkanRenderImpl> renderImpl;

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