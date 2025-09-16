#include "VulkanCommandBufferPool.h"
#include "VulkanCommandBuffer.h"

#include "Core/Logger.h"
namespace sh::render::vk
{
	VulkanCommandBufferPool::VulkanCommandBufferPool(const VulkanContext& context, uint32_t graphicsQueueFamily, uint32_t transferQueueFamily) :
		context(context), graphicsQueueFamily(graphicsQueueFamily), transferQueueFamily(transferQueueFamily)
	{

	}
	VulkanCommandBufferPool::~VulkanCommandBufferPool()
	{
		std::unique_lock<std::shared_mutex> ulock{ mu };
		allocated.clear();

		for (auto& td : threadDatas)
		{
			for (auto& cmd : td.commands)
			{
				while (!cmd.cmds.empty())
					cmd.cmds.pop(); // unique_ptr 소멸
				if (cmd.cmdPool != VK_NULL_HANDLE)
				{
					vkDestroyCommandPool(context.GetDevice(), cmd.cmdPool, nullptr);
					cmd.cmdPool = VK_NULL_HANDLE;
				}
			}
		}
		threadDatas.clear();
	}
	SH_RENDER_API auto VulkanCommandBufferPool::AllocateCommandBuffer(std::thread::id thr, VkQueueFlagBits queueType) -> VulkanCommandBuffer*
	{
		{
			std::shared_lock<std::shared_mutex> slock{ mu };
			for (auto& threadData : threadDatas)
			{
				if (threadData.id == thr)
				{
					QueueType type;
					PerThreadData::Command* command = nullptr;
					if (queueType == VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
						type = QueueType::Grahpics;
					else if (queueType == VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT)
						type = QueueType::Transfer;
					else
					{
						assert(false);
						SH_ERROR_FORMAT("Incorrect queueType: {}", string_VkQueueFlagBits(queueType));
						return nullptr;
					}

					command = &threadData.commands[type];

					// 풀과 버퍼 존재
					if (command->cmdPool != VK_NULL_HANDLE)
					{
						if (!command->cmds.empty())
						{
							auto uptr = std::move(command->cmds.top());
							command->cmds.pop();
							VulkanCommandBuffer* cmd = uptr.get();

							slock.unlock();
							std::unique_lock<std::shared_mutex> ulock{ mu };
							allocated.emplace(cmd, AllocData{ std::move(uptr), thr, type });
							return cmd;
						}
						else
						{
							// 풀은 있으나 사용 가능한 커맨드 버퍼가 없음
							return nullptr;
						}
					}
					else
					{
						// 풀이 아직 없는 경우
						slock.unlock();
						std::unique_lock<std::shared_mutex> ulock{ mu };

						// 락 사이에 다른 쓰레드가 이미 만들었을 수도 있으니 재검사
						if (command->cmdPool == VK_NULL_HANDLE)
						{
							VkCommandPool pool = CreateCommandPool(queueType);
							if (pool == VK_NULL_HANDLE)
								return nullptr;

							command->cmdPool = pool;
							for (int i = 0; i < cap; ++i)
							{
								auto cmdBuffer = std::make_unique<VulkanCommandBuffer>(context);
								cmdBuffer->Create(pool);

								command->cmds.push(std::move(cmdBuffer));
							}
						}
						// 하나 꺼내서 allocated로 이동
						auto uptr = std::move(command->cmds.top());
						command->cmds.pop();
						VulkanCommandBuffer* cmd = uptr.get();
						allocated.emplace(cmd, AllocData{ std::move(uptr), thr, type });
						return cmd;
					}
				}
			}
			// 여기까지 왔으면 해당 스레드 데이터가 없는거임
		}
		{
			std::unique_lock<std::shared_mutex> ulock{ mu };
			// 재검사(다른 스레드가 추가했을 수도 있으므로)
			for (auto& threadData : threadDatas)
			{
				if (threadData.id == thr)
				{
					// 이미 추가됨 -> 재호출하여 기존 로직 사용
					ulock.unlock();
					return AllocateCommandBuffer(thr, queueType);
				}
			}
			PerThreadData newData;
			newData.id = thr;
			threadDatas.push_back(std::move(newData));
			auto& threadData = threadDatas.back();

			QueueType type;
			PerThreadData::Command* command = nullptr;
			if (queueType == VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
				type = QueueType::Grahpics;
			else if (queueType == VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT)
				type = QueueType::Transfer;
			else
			{
				assert(false);
				SH_ERROR_FORMAT("Incorrect queueType: {}", string_VkQueueFlagBits(queueType));
				return nullptr;
			}
			command = &threadData.commands[type];

			// 풀 생성 및 커맨드 버퍼들 준비
			VkCommandPool pool = CreateCommandPool(queueType);
			if (pool == VK_NULL_HANDLE)
				return nullptr;

			command->cmdPool = pool;
			for (int i = 0; i < cap; ++i)
			{
				auto cmdBuffer = std::make_unique<VulkanCommandBuffer>(context);
				cmdBuffer->Create(pool);

				command->cmds.push(std::move(cmdBuffer));
			}

			// 하나 꺼내서 allocated로 이동
			auto uptr = std::move(command->cmds.top());
			command->cmds.pop();
			VulkanCommandBuffer* cmd = uptr.get();
			allocated.emplace(cmd, AllocData{ std::move(uptr), thr, type });

			return cmd;
		}
	}
	SH_RENDER_API void VulkanCommandBufferPool::DeallocateCommandBuffer(VulkanCommandBuffer& cmdBuffer)
	{
		std::unique_lock<std::shared_mutex> ulock{ mu };
		auto it = allocated.find(&cmdBuffer);
		if (it == allocated.end())
		{
			SH_ERROR("Attempt to deallocate unknown command buffer");
			return;
		}
		AllocData ad = std::move(it->second);
		allocated.erase(it);

		for (auto& td : threadDatas)
		{
			if (td.id == ad.tid)
			{
				ad.ptr->ResetCommand();
				ad.ptr->ResetSyncObjects();
				auto& command = td.commands[ad.type];
				command.cmds.push(std::move(ad.ptr));
				return;
			}
		}
	}
	auto VulkanCommandBufferPool::CreateCommandPool(VkQueueFlagBits queueType) -> VkCommandPool
	{
		VkCommandPool pool = nullptr;

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.pNext = nullptr;
		if (queueType == VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
			poolInfo.queueFamilyIndex = graphicsQueueFamily;
		else if (queueType == VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT)
			poolInfo.queueFamilyIndex = transferQueueFamily;
		else
		{
			assert(false);
			SH_ERROR_FORMAT("Incorrect queueType: {}", string_VkQueueFlagBits(queueType));
			return nullptr;
		}

		poolInfo.flags = VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; //명령 버퍼가 개별적으로 기록되도록 허용
		auto result = vkCreateCommandPool(context.GetDevice(), &poolInfo, nullptr, &pool);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
		{
			SH_ERROR_FORMAT("Failed to create VkCommandPool!: {}", string_VkResult(result));
			return nullptr;
		}
		return pool;
	}
}//namespace