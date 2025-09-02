#pragma once

#include "Export.h"
#include "Core/NonCopyable.h"

#include "VulkanConfig.h"

#include <functional>
#include <initializer_list>

namespace sh::render::vk 
{
	class VulkanContext;
	class VulkanCommandBuffer : public core::INonCopyable
	{
	public:
		struct SemaphoreInfo
		{
			VkSemaphore semaphore = VK_NULL_HANDLE;
			uint64_t semaphoreValue = 0;
			bool bTimelineSemaphore = false;
		};
	public:
		SH_RENDER_API VulkanCommandBuffer(const VulkanContext& context);
		SH_RENDER_API VulkanCommandBuffer(VulkanCommandBuffer&& other) noexcept;
		SH_RENDER_API ~VulkanCommandBuffer();

		SH_RENDER_API void SetWaitStage(const std::initializer_list<VkPipelineStageFlagBits> stages);
		SH_RENDER_API auto GetWaitStage() const -> VkPipelineStageFlags;

		SH_RENDER_API void SetWaitSemaphore(const SemaphoreInfo& semaphore);
		SH_RENDER_API void SetWaitSemaphore(const std::initializer_list<SemaphoreInfo>& s);
		SH_RENDER_API void SetWaitSemaphore(const std::vector<SemaphoreInfo>& semaphores);
		SH_RENDER_API void SetWaitSemaphore(std::vector<SemaphoreInfo>&& semaphores);
		SH_RENDER_API auto GetWaitSemaphores() const -> const std::vector<SemaphoreInfo>&;

		SH_RENDER_API void SetSignalSemaphore(const SemaphoreInfo& semaphore);
		SH_RENDER_API void SetSignalSemaphore(const std::initializer_list<SemaphoreInfo>& s);
		SH_RENDER_API void SetSignalSemaphore(const std::vector<SemaphoreInfo>& semaphores);
		SH_RENDER_API void SetSignalSemaphore(std::vector<SemaphoreInfo>&& semaphores);
		SH_RENDER_API auto GetSignalSemaphores() const -> const std::vector<SemaphoreInfo>&;

		SH_RENDER_API auto Build(const std::function<void()>& commands, bool bUsingOnce = false) -> VkResult;

		SH_RENDER_API auto Create(VkCommandPool cmdPool, const VkCommandBufferAllocateInfo* info = nullptr) -> VkResult;
		SH_RENDER_API auto GetFence() const -> VkFence;
		/// @brief 커맨드에 기록된 내용을 지우는 함수.
		SH_RENDER_API auto Reset() -> VkResult;
		/// @brief 세마포어와 waitStage를 초기화 하는 함수.
		SH_RENDER_API void ResetSyncObjects();
		/// @brief 커맨드 버퍼와 동기화 객체들을 모두 삭제하는 함수.
		SH_RENDER_API void Clear();

		SH_RENDER_API auto GetCommandBuffer() const-> VkCommandBuffer;
	private:
		auto Begin(bool bOnce) -> VkResult;
		auto End() -> VkResult;
	private:
		const VulkanContext& context;
		VkCommandBuffer buffer = nullptr;
		VkCommandPool cmdPool = nullptr;

		VkPipelineStageFlags waitStage;
		std::vector<SemaphoreInfo> waitSemaphores;
		std::vector<SemaphoreInfo> signalSemaphores;

		mutable VkFence fence = nullptr;
	};
}