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
		std::lock_guard<std::mutex> lock(mu);
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
		std::lock_guard<std::mutex> lock(mu);

		auto toTypeFn = 
			[&](VkQueueFlagBits qt) -> QueueType
			{
				if (qt == VK_QUEUE_GRAPHICS_BIT) 
					return QueueType::Graphics;
				if (qt == VK_QUEUE_TRANSFER_BIT)  
					return QueueType::Transfer;

				SH_ERROR_FORMAT("Incorrect queueType: {}", string_VkQueueFlagBits(qt));
				assert(false);
				return QueueType::Graphics;
			};
		QueueType type = toTypeFn(queueType);

		// thread 데이터 찾기 (없으면 생성)
		PerThreadData* td = nullptr;
		for (auto& t : threadDatas)
		{
			if (t.id == thr) 
			{ 
				td = &t;
				break; 
			}
		}
		if (td == nullptr)
		{
			threadDatas.emplace_back();
			td = &threadDatas.back();
			td->id = thr;
		}

		PerThreadData::Command& command = td->commands[type];

		// pool이 없으면 생성하고 미리 할당
		if (command.cmdPool == VK_NULL_HANDLE)
		{
			VkCommandPool pool = CreateCommandPool(queueType);
			if (pool == VK_NULL_HANDLE) 
				return nullptr;

			command.cmdPool = pool;
			for (int i = 0; i < cap; ++i)
			{
				auto cmdBuffer = std::make_unique<VulkanCommandBuffer>(context);
				cmdBuffer->Create(pool);
				command.cmds.push(std::move(cmdBuffer));
			}
		}

		// 커맨드 버퍼가 비었으면 추가 생성
		if (command.cmds.empty())
		{
			for (int i = 0; i < cap; ++i)
			{
				auto cmdBuffer = std::make_unique<VulkanCommandBuffer>(context);
				cmdBuffer->Create(command.cmdPool);
				command.cmds.push(std::move(cmdBuffer));
			}
		}

		std::unique_ptr<VulkanCommandBuffer> cmdPtr = std::move(command.cmds.top());
		command.cmds.pop();

		VulkanCommandBuffer* rawCmdPtr = cmdPtr.get();
		allocated.emplace(rawCmdPtr, AllocData{ std::move(cmdPtr), thr, type });
		return rawCmdPtr;
	}
	SH_RENDER_API void VulkanCommandBufferPool::DeallocateCommandBuffer(VulkanCommandBuffer& cmdBuffer)
	{
		std::lock_guard<std::mutex> lock(mu);

		auto it = allocated.find(&cmdBuffer);
		if (it == allocated.end())
		{
			SH_ERROR("Attempt to deallocate unknown command buffer");
			assert(false);
			return;
		}

		AllocData ad = std::move(it->second);
		allocated.erase(it);

		// 원래 스레드 데이터 찾아서 반환
		for (auto& td : threadDatas)
		{
			if (td.id == ad.tid)
			{
				ad.ptr->ResetCommand();
				ad.ptr->ResetSyncObjects();
				td.commands[ad.type].cmds.push(std::move(ad.ptr));
				return;
			}
		}

		SH_ERROR("Failed to find owner thread data for command buffer; dropping it");
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