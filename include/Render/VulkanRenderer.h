﻿#pragma once
#pragma warning(disable: 4251)

#include "Renderer.h"

#include <Core/NonCopyable.h>
#include "VulkanImpl/VulkanConfig.h"

#include "../vma-src/include/vk_mem_alloc.h"
#include "glm/mat4x4.hpp"

#include <string>
#include <vector>
#include <array>
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
		public Renderer, public sh::core::INonCopyable{
	public:
		static constexpr int MAX_FRAME_DRAW = 2;
	private:
		sh::window::Window* window;
		sh::window::WinHandle winHandle;

		std::vector<const char*> requestedLayer;
		std::vector<const char*> requestedInstanceExtension;
		std::vector<const char*> requestedDeviceExtension;

		std::unique_ptr<impl::VulkanSurface> surface;
		std::unique_ptr<impl::VulkanLayer> layers;
		std::array<std::unique_ptr<impl::VulkanCommandBuffer>, MAX_FRAME_DRAW> cmdBuffers;

		VkInstance instance;
		VkPhysicalDeviceProperties gpuProp;
		VkPhysicalDevice gpu;
		VkDevice device; //논리적 장치
		std::vector<impl::VulkanFramebuffer> framebuffers;

		VkCommandPool cmdPool;
		VkDescriptorPool descPool;

		std::vector<VkPhysicalDevice> gpus;
		std::vector<VkQueueFamilyProperties> queueFamilies;

		VkDebugUtilsMessengerEXT debugMessenger;
		std::string validationLayerName;

		uint32_t graphicsQueueIndex;
		uint32_t surfaceQueueIndex;
		VkQueue graphicsQueue;
		VkQueue surfaceQueue;

		std::array<VkSemaphore, MAX_FRAME_DRAW> imageAvailableSemaphore;
		std::array<VkSemaphore, MAX_FRAME_DRAW> renderFinishedSemaphore;
		std::array<VkFence, MAX_FRAME_DRAW> inFlightFence;

		int currentFrame;
		uint32_t descriptorPoolSize;

		VmaAllocator allocator;

		bool isInit : 1;
		bool bPause : 1;
		bool bFindValidationLayer : 1;
		const bool bEnableValidationLayers : 1;
		bool bReCreateDescriptorPool : 1;
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

		auto CreateSyncObjects() -> VkResult;
		void DestroySyncObjects();

		auto CreateDescriptorPool() -> VkResult;
		void DestroyDescriptorPool();

		void CreateAllocator();
		void DestroyAllocator();

		void PrintLayer();
	public:
		SH_RENDER_API VulkanRenderer();
		SH_RENDER_API ~VulkanRenderer();

		SH_RENDER_API bool Init(sh::window::Window& win) override;
		SH_RENDER_API bool Resizing() override;
		SH_RENDER_API void Clean() override;

		SH_RENDER_API bool IsInit() const override;

		SH_RENDER_API void Render(float deltaTime) override;
		SH_RENDER_API void Pause(bool b) override;

		SH_RENDER_API void ReAllocateDesriptorPool();
		SH_RENDER_API void ReAllocateSamplerDesriptorPool();

		SH_RENDER_API auto GetDevice() const -> VkDevice;
		SH_RENDER_API auto GetGPU() const -> VkPhysicalDevice;
		SH_RENDER_API auto GetCommandPool() const -> VkCommandPool;
		SH_RENDER_API auto GetCommandBuffer() const -> VkCommandBuffer;
		SH_RENDER_API auto GetGraphicsQueue() const -> VkQueue;
		SH_RENDER_API auto GetMainFramebuffer() const -> const Framebuffer* override;
		SH_RENDER_API auto GetDescriptorPool() const -> VkDescriptorPool;
		SH_RENDER_API auto GetCurrentFrame() const -> int;
		SH_RENDER_API auto GetWidth() const -> uint32_t override;
		SH_RENDER_API auto GetHeight() const -> uint32_t override;
		SH_RENDER_API auto GetAllocator() const -> VmaAllocator;
		SH_RENDER_API auto GetGPUProperty() const -> const VkPhysicalDeviceProperties&;
	};
}//namespace
