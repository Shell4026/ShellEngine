#pragma once
#include "../Export.h"
#include "VulkanConfig.h"

#include "Core/ISyncable.h"
#include "Core/NonCopyable.h"
#include "Core/SpinLock.h"

#include <utility>
#include <optional>
#include <vector>
#include <mutex>
namespace sh::render::vk
{
	class VulkanContext;
	class VulkanCommandBuffer;

	/// @brief 스레드 안전한 Vulkan 큐 매니저
	class VulkanQueueManager : public core::INonCopyable
	{
	public:
		struct QueueFamily
		{
			uint32_t idx = 0;
			uint32_t queueCount = 0;
		};
	public:
		SH_RENDER_API VulkanQueueManager(const VulkanContext& ctx);
		SH_RENDER_API VulkanQueueManager(VulkanQueueManager&& other) noexcept;
		SH_RENDER_API ~VulkanQueueManager();

		SH_RENDER_API void QueryQueueFamily(VkSurfaceKHR surface);
		SH_RENDER_API auto GetGraphicsQueueFamily() const -> QueueFamily;
		SH_RENDER_API auto GetTransferQueueFamily() const -> QueueFamily;
		SH_RENDER_API auto GetSurfaceQueueFamily() const -> QueueFamily;

		SH_RENDER_API void CreateGraphicsQueue();
		SH_RENDER_API void CreateTransferQueue();
		SH_RENDER_API void CreateSurfaceQueue(VkSurfaceKHR surface);

		SH_RENDER_API auto GetGraphicsQueue() const -> VkQueue;
		SH_RENDER_API auto GetTransferQueue() const -> VkQueue;
		SH_RENDER_API auto GetSurfaceQueue() const -> VkQueue;

		/// @brief 큐에 커맨드를 제출한다.
		/// @param queue 큐
		/// @param cmd 커맨드 버퍼
		/// @param fence 펜스
		SH_RENDER_API void SubmitCommand(VkQueue queue, const VulkanCommandBuffer& cmd, VkFence fence = nullptr);
	private:
		void GetQueueFamilyProperties();
		auto SelectQueueFamily(VkQueueFlagBits queueType) const -> std::optional<QueueFamily>;
		auto GetSurfaceQueueFamily(VkSurfaceKHR surface) const -> std::optional<QueueFamily>;
	private:
		const VulkanContext& ctx;

		std::vector<VkQueueFamilyProperties> queueFamilyProps;

		QueueFamily graphicsQueueFamily;
		QueueFamily surfaceQueueFamily;
		QueueFamily transferQueueFamily;

		VkQueue graphicsQueue = nullptr;
		VkQueue surfaceQueue = nullptr;
		VkQueue transferQueue = nullptr;

		core::SpinLock spinLock;
	};
}//namespace