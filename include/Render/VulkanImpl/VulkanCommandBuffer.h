#pragma once

#include "Export.h"
#include "Core/NonCopyable.h"

#include "VulkanConfig.h"

#include <functional>
#include <initializer_list>

namespace sh::render::impl {
	class VulkanCommandBuffer : public core::INonCopyable
	{
	private:
		VkCommandBuffer buffer;

		VkDevice device;
		VkCommandPool cmdPool;

		VkPipelineStageFlags waitStage;
		std::vector<VkSemaphore> waitSemaphores;
		std::vector<VkSemaphore> signalSemaphores;
	private:
		auto Begin(VkCommandBufferBeginInfo* info = nullptr) -> VkResult;
		auto End() -> VkResult;
	public:
		SH_RENDER_API VulkanCommandBuffer(VkDevice device, VkCommandPool pool);
		SH_RENDER_API VulkanCommandBuffer(VulkanCommandBuffer&& other) noexcept;
		SH_RENDER_API ~VulkanCommandBuffer();

		SH_RENDER_API void SetWaitStage(const std::initializer_list<VkPipelineStageFlagBits> stages);

		SH_RENDER_API void SetWaitSemaphore(const std::initializer_list<VkSemaphore> s);
		SH_RENDER_API void SetSignalSemaphore(const std::initializer_list<VkSemaphore> s);

		SH_RENDER_API auto Submit(VkQueue queue, const std::function<void()>& commands, VkCommandBufferBeginInfo* beginInfo = nullptr, VkFence fence = nullptr) -> VkResult;

		SH_RENDER_API auto Create(const VkCommandBufferAllocateInfo* info = nullptr) -> VkResult;
		SH_RENDER_API auto Reset() -> VkResult;
		SH_RENDER_API void Clean();

		SH_RENDER_API auto GetCommandBuffer() const-> VkCommandBuffer;
	};
}