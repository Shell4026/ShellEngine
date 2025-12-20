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
	class VulkanCommandBuffer;

	class VulkanRenderer :
		public Renderer, public sh::core::INonCopyable
	{
	public:
		SH_RENDER_API VulkanRenderer();
		SH_RENDER_API ~VulkanRenderer();

		SH_RENDER_API bool Init(sh::window::Window& win) override;
		SH_RENDER_API bool Resizing() override;
		SH_RENDER_API void Clear() override;

		SH_RENDER_API bool IsInit() const override;

		SH_RENDER_API void Render() override;

		SH_RENDER_API void WaitForCurrentFrame() override;

		SH_RENDER_API auto GetCurrentFrame() const -> int;
		SH_RENDER_API auto GetWidth() const -> uint32_t override;
		SH_RENDER_API auto GetHeight() const -> uint32_t override;
		
		SH_RENDER_API auto GetTimelineSemaphore() const -> VkSemaphore;
		/// @brief [원자적] 첫번째 패스의 타임라인 세마포어의 값을 가져온다.
		/// @return 타임라인 세마포어 값
		SH_RENDER_API auto GetTimelineValue() const -> uint64_t;

		SH_RENDER_API auto GetContext() const -> IRenderContext* override;

		SH_RENDER_API void Sync() override;
	protected:
		void OnCameraAdded(const Camera* camera) override;
		void OnCameraRemoved(const Camera* camera) override;
	private:
		auto CreateSyncObjects() -> VkResult;
		void DestroySyncObjects();
	private:
		std::unique_ptr<VulkanContext> context;

		VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
		VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;
		VkSemaphore timelineSemaphore = VK_NULL_HANDLE;

		uint64_t timelineValue = 0;
		std::atomic<uint64_t> timelineValueAtomic = 0;

		std::vector<VkFence> frameFences;
		VkFence inFlightFence;

		VulkanCameraBuffers* camManager = nullptr;

		int currentFrame;

		bool isInit = false;
	};
}//namespace
