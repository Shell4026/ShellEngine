#pragma once
#include "Render/Renderer.h"
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
	class VulkanContext;
	class VulkanCommandBuffer;

	class VulkanRenderer :
		public Renderer, public sh::core::INonCopyable
	{
	public:
		SH_RENDER_API VulkanRenderer();
		SH_RENDER_API ~VulkanRenderer();

		SH_RENDER_API void CreateContext(const window::Window& win) override;
		SH_RENDER_API void DestroyContext() override;
		SH_RENDER_API bool Init(window::Window& win) override;
		SH_RENDER_API bool Resizing() override;
		SH_RENDER_API void Clear() override;

		SH_RENDER_API bool IsInit() const override { return isInit; }

		SH_RENDER_API void Render() override;

		SH_RENDER_API void WaitForCurrentFrame() override;

		SH_RENDER_API auto GetWidth() const -> uint32_t override;
		SH_RENDER_API auto GetHeight() const -> uint32_t override;

		SH_RENDER_API void Sync() override;

		SH_RENDER_API auto GetContext() const -> IRenderContext* override;
		auto GetTimelineSemaphore() const -> VkSemaphore { return timelineSemaphore; }
		auto GetCurrentFrame() const -> int { return currentFrame; }
	private:
		auto CreateSyncObjects() -> VkResult;
		void DestroySyncObjects();
	private:
		std::unique_ptr<VulkanContext> context;

		VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
		VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;
		VkSemaphore timelineSemaphore = VK_NULL_HANDLE;

		std::vector<VkFence> frameFences;
		VkFence inFlightFence;

		int currentFrame;

		bool isInit = false;
	};
}//namespace
