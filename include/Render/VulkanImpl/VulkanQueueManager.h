#pragma once
#include "Export.h"
#include "VulkanConfig.h"

#include "Core/ISyncable.h"
#include "Core/NonCopyable.h"
#include "Core/SpinLock.h"

#include <utility>
#include <optional>
#include <vector>

namespace sh::render::vk
{
	class VulkanCommandBuffer;

	/// @brief 스레드 안전한 Vulkan 큐 매니저
	class VulkanQueueManager : public core::INonCopyable
	{
	private:
		VkDevice device = nullptr;
		VkPhysicalDevice gpu = nullptr;

		std::vector<VkQueueFamilyProperties> queueFamilyProps;

		uint8_t graphicsQueueFamilyIdx = 0;
		uint8_t surfaceQueueFamilyIdx = 0;
		uint8_t transferQueueFamilyIdx = 0;

		VkQueue graphicsQueue = nullptr;
		VkQueue surfaceQueue = nullptr;
		VkQueue transferQueue = nullptr; // 큐 하나만 지원하면 nullptr

		core::SpinLock spinLock;
	private:
		void GetQueueFamilyProperties();
		auto SelectQueueFamily(VkQueueFlagBits queueType) -> std::optional<int>;
		auto GetSurfaceQueueFamily(VkSurfaceKHR surface) -> std::optional<int>;
	public:
		SH_RENDER_API VulkanQueueManager(VkPhysicalDevice gpu);
		SH_RENDER_API VulkanQueueManager(VulkanQueueManager&& other) noexcept;
		SH_RENDER_API ~VulkanQueueManager();

		SH_RENDER_API void SetDevice(VkDevice device);

		SH_RENDER_API auto GetGraphicsQueueFamilyIdx() const -> uint8_t;
		SH_RENDER_API auto GetTransferQueueFamilyIdx() const -> uint8_t;
		SH_RENDER_API auto GetSurfaceQueueFamilyIdx() const -> uint8_t;

		SH_RENDER_API void CreateGraphicsQueue();
		SH_RENDER_API void CreateTransferQueue();
		SH_RENDER_API void CreateSurfaceQueue(VkSurfaceKHR surface);

		SH_RENDER_API auto GetGraphicsQueue() const -> VkQueue;
		SH_RENDER_API auto GetTransferQueue() const -> VkQueue;
		SH_RENDER_API auto GetSurfaceQueue() const -> VkQueue;

		SH_RENDER_API void SubmitCommand(const VulkanCommandBuffer& cmd, VkFence fence = nullptr);
	};
}//namespace