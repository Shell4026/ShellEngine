#pragma once
#include "../Export.h"
#include "VulkanContext.h"

#include <vector>
#include <thread>
#include <stack>
#include <map>
#include <array>
#include <memory>
#include <mutex>
#include <unordered_map>
namespace sh::render::vk
{
	class VulkanCommandBuffer;
	/// @brief 스레드별로 커맨드 풀을 가지고, 미리 생성된 커맨드 버퍼를 제공 해주는 클래스. 스레드 안전하다.
	class VulkanCommandBufferPool
	{
	public:
		SH_RENDER_API VulkanCommandBufferPool(const VulkanContext& context, uint32_t graphicsQueueFamily, uint32_t transferQueueFamily);
		SH_RENDER_API ~VulkanCommandBufferPool();

		/// @brief 해당 스레드에서 큐 타입에 맞는 커맨드 버퍼를 할당 받는다.
		/// @param thr 스레드 아이디
		/// @param queueType 큐 타입
		/// @return 없다면 nullptr을 반환
		SH_RENDER_API auto AllocateCommandBuffer(std::thread::id thr, VkQueueFlagBits queueType) -> VulkanCommandBuffer*;
		/// @brief 할당 받았던 커맨드 버퍼를 반환 하고 상태를 리셋한다.
		/// @param cmdBuffer 커맨드 버퍼
		SH_RENDER_API void DeallocateCommandBuffer(VulkanCommandBuffer& cmdBuffer);
	private:
		auto CreateCommandPool(VkQueueFlagBits queueType) -> VkCommandPool;
	private:
		const VulkanContext& context;
		const uint32_t graphicsQueueFamily;
		const uint32_t transferQueueFamily;

		enum QueueType
		{
			Graphics = 0,
			Transfer = 1
		};

		struct PerThreadData
		{
			PerThreadData() = default;
			PerThreadData(PerThreadData&& other) noexcept :
				id(other.id)
			{
				for (int i = 0; i < commands.size(); ++i)
					commands[i] = std::move(other.commands[i]);
			}
			struct Command
			{
				Command() = default;
				Command(Command&& other) noexcept :
					cmdPool(other.cmdPool), cmds(std::move(other.cmds))
				{}
				auto operator=(Command&& other) noexcept -> Command&
				{
					cmdPool = other.cmdPool;
					cmds = std::move(other.cmds);
					return *this;
				}

				VkCommandPool cmdPool = nullptr;
				std::stack<std::unique_ptr<VulkanCommandBuffer>> cmds;
			};

			std::thread::id id;
			std::array<Command, 2> commands;
		};
		
		struct AllocData
		{
			std::unique_ptr<VulkanCommandBuffer> ptr; // 현재 빌려간 커맨드 버퍼의 소유권을 갖는다.
			std::thread::id tid;
			QueueType type;
		};
		std::vector<PerThreadData> threadDatas;
		std::unordered_map<VulkanCommandBuffer*, AllocData> allocated;
		std::mutex mu;

		int cap = 8;
	};
}//namespace