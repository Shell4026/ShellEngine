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
	private:
		const VulkanContext& context;
		VkCommandBuffer buffer;
		VkCommandPool cmdPool;

		VkPipelineStageFlags waitStage;
		std::vector<VkSemaphore> waitSemaphores;
		std::vector<VkSemaphore> signalSemaphores;

		VkQueueFlagBits queueType = VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT;
	private:
		auto Begin(VkCommandBufferBeginInfo* info = nullptr) -> VkResult;
		auto End() -> VkResult;
	public:
		SH_RENDER_API VulkanCommandBuffer(const VulkanContext& context, VkQueueFlagBits queueType = VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT);
		SH_RENDER_API VulkanCommandBuffer(VulkanCommandBuffer&& other) noexcept;
		SH_RENDER_API ~VulkanCommandBuffer();

		SH_RENDER_API void SetWaitStage(const std::initializer_list<VkPipelineStageFlagBits> stages);
		SH_RENDER_API auto GetWaitStage() const -> VkPipelineStageFlags;

		SH_RENDER_API void SetWaitSemaphore(const std::initializer_list<VkSemaphore> s);
		SH_RENDER_API auto GetWaitSemaphore() const -> const std::vector<VkSemaphore>&;

		SH_RENDER_API void SetSignalSemaphore(const std::initializer_list<VkSemaphore> s);
		SH_RENDER_API auto GetSignalSemaphore() const -> const std::vector<VkSemaphore>&;

		SH_RENDER_API auto Build(const std::function<void()>& commands, VkCommandBufferBeginInfo* beginInfo = nullptr) -> VkResult;

		SH_RENDER_API auto Create(VkCommandPool cmdPool, const VkCommandBufferAllocateInfo* info = nullptr) -> VkResult;
		SH_RENDER_API auto Reset() -> VkResult;
		SH_RENDER_API void Clear();

		SH_RENDER_API auto GetCommandBuffer() const-> VkCommandBuffer;

		SH_RENDER_API auto GetQueueType() const -> VkQueueFlagBits;
	};
}