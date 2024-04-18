#pragma once
#pragma warning(disable: 4251)

#include "Renderer.h"

#include <Core/NonCopyable.h>
#include "VulkanImpl/VulkanConfig.h"

#include <string>
#include <vector>
#include <optional>
#include <functional>
#include <memory>

namespace sh::render {
	namespace impl
	{
		class VulkanLayer;
		class VulkanSurface;
		class VulkanPipeline;
		class VulkanCommandBuffer;
		class VulkanFramebuffer;
	}
	class VulkanRenderer :
		public IRenderer, public sh::core::INonCopyable{
	private:
		sh::window::Window* window;
		sh::window::WinHandle winHandle;

		std::vector<const char*> requestedLayer;
		std::vector<const char*> requestedExtension;

		std::unique_ptr<impl::VulkanSurface> surface;
		std::unique_ptr<impl::VulkanLayer> layers;
		std::unique_ptr<impl::VulkanPipeline> pipeline;
		std::unique_ptr<impl::VulkanCommandBuffer> cmdBuffer;

		VkInstance instance;
		VkPhysicalDevice gpu;
		VkDevice device; //논리적 장치
		std::vector<impl::VulkanFramebuffer> framebuffers;

		VkCommandPool cmdPool;

		std::vector<VkPhysicalDevice> gpus;
		std::vector<VkQueueFamilyProperties> queueFamilies;

		VkDebugUtilsMessengerEXT debugMessenger;
		std::string validationLayerName;

		uint32_t graphicsQueueIndex;
		uint32_t surfaceQueueIndex;
		VkQueue graphicsQueue;
		VkQueue surfaceQueue;

		VkSemaphore imageAvailableSemaphore;
		VkSemaphore renderFinishedSemaphore;
		VkFence inFlightFence;

		bool isInit : 1;
		bool bPause : 1;
		bool bFindValidationLayer : 1;
		const bool bEnableValidationLayers : 1;
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

		auto CreateCommandPool(uint32_t queue) -> VkResult;
		void DestroyCommandPool();
		auto ResetCommandPool(uint32_t queue) -> VkResult;

		auto CreateSyncObjects()->VkResult;
		void DestroySyncObjects();

		void PrintLayer();
	public:
		SH_RENDER_API VulkanRenderer();
		SH_RENDER_API ~VulkanRenderer();

		SH_RENDER_API bool Init(sh::window::Window& win) override;
		SH_RENDER_API void Clean() override;

		SH_RENDER_API bool IsInit() const override;

		SH_RENDER_API void Render() override;
		SH_RENDER_API void Pause(bool b) override;
	};
}//namespace
