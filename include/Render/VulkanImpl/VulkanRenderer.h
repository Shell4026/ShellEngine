#pragma once

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
	class VulkanContext;
	class VulkanCameraBuffers;

	class VulkanRenderer :
		public Renderer, public sh::core::INonCopyable
	{
	private:
		std::unique_ptr<VulkanContext> context;

		VkSemaphore imageAvailableSemaphore;
		VkSemaphore renderFinishedSemaphore;
		VkSemaphore gameThreadSemaphore;
		VkFence inFlightFence;

		VulkanCameraBuffers* camManager = nullptr;

		int currentFrame;

		bool isInit = false;
	private:
		auto CreateSyncObjects() -> VkResult;
		void DestroySyncObjects();
	protected:
		void OnCameraAdded(const Camera* camera) override;
		void OnCameraRemoved(const Camera* camera) override;
	public:
		SH_RENDER_API VulkanRenderer();
		SH_RENDER_API ~VulkanRenderer();

		SH_RENDER_API bool Init(const sh::window::Window& win) override;
		SH_RENDER_API bool Resizing() override;
		SH_RENDER_API void Clear() override;

		SH_RENDER_API bool IsInit() const override;

		SH_RENDER_API void Render() override;

		SH_RENDER_API void WaitForCurrentFrame();

		SH_RENDER_API auto GetCurrentFrame() const -> int;
		SH_RENDER_API auto GetWidth() const -> uint32_t override;
		SH_RENDER_API auto GetHeight() const -> uint32_t override;
		
		SH_RENDER_API auto GetRenderFinshedSemaphore() const -> VkSemaphore;
		SH_RENDER_API auto GetGameThreadSemaphore() const -> VkSemaphore;

		SH_RENDER_API auto GetContext() const -> IRenderContext* override;

		SH_RENDER_API void Sync() override;
	};
}//namespace
