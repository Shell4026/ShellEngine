#include "VulkanDescriptorPool.h"

#include <array>
#include <stdexcept>
#include <cassert>

namespace sh::render::vk
{
	VulkanDescriptorPool::VulkanDescriptorPool(VkDevice device, size_t size) :
		device(device),

		initialSize(size), size(size)
	{
	}
	VulkanDescriptorPool::VulkanDescriptorPool(VulkanDescriptorPool&& other) noexcept :
		device(other.device),

		initialSize(other.initialSize), size(other.size),
		readyPool(std::move(other.readyPool)), fullPool(std::move(other.fullPool))
	{
		other.device = VK_NULL_HANDLE;
		other.size = 0;
		other.initialSize = 0;
	}
	VulkanDescriptorPool::~VulkanDescriptorPool()
	{
		Clean();
	}

	auto VulkanDescriptorPool::GetPool() -> Pool&
	{
		if (readyPool.empty())
		{
			std::array<VkDescriptorPoolSize, 2> poolSizes{};
			poolSizes[0].type = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSizes[0].descriptorCount = 1;
			poolSizes[1].type = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSizes[1].descriptorCount = 1;

			VkDescriptorPoolCreateInfo poolInfo{};
			poolInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.poolSizeCount = poolSizes.size();
			poolInfo.pPoolSizes = poolSizes.data();
			poolInfo.maxSets = size;
			poolInfo.flags = VkDescriptorPoolCreateFlagBits::VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

			VkDescriptorPool descPool;
			auto result = vkCreateDescriptorPool(device, &poolInfo, nullptr, &descPool);
			assert(result == VK_SUCCESS);
			readyPool.push(Pool{ descPool, false });

			size = static_cast<std::size_t>(size * 1.5f);
			if (size > 4096) 
				size = 4096;

			return readyPool.top();
		}
		return readyPool.top();
	}

	auto VulkanDescriptorPool::AllocateDescriptorSet(VkDescriptorSetLayout layouts, uint32_t count) -> VkDescriptorSet
	{
		Pool pool = GetPool();
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = pool.pool;
		allocInfo.descriptorSetCount = count;
		allocInfo.pSetLayouts = &layouts;

		VkDescriptorSet descriptorSet;
		auto result = vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);
		if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL)
		{
			pool.full = true;
			readyPool.pop();
			fullPool.insert(pool);

			pool = GetPool();
			allocInfo.descriptorPool = pool.pool;
			result = vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);
			if (result != VkResult::VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create descriptor pool.");
			}
		}

		allocated.insert({ descriptorSet, pool });

		return descriptorSet;
	}

	void VulkanDescriptorPool::Clean()
	{
		allocated.clear();
		size = initialSize;
		for (auto& pool : fullPool)
			vkDestroyDescriptorPool(device, pool.pool, nullptr);
		fullPool.clear();

		while (!readyPool.empty())
		{
			vkDestroyDescriptorPool(device, readyPool.top().pool, nullptr);
			readyPool.pop();
		}
	}
	void VulkanDescriptorPool::FreeDescriptorSet(VkDescriptorSet descSet)
	{
		auto it = allocated.find(descSet);
		if (it == allocated.end())
			return;

		vkFreeDescriptorSets(device, it->second.pool, 1, &it->first);
		if (it->second.full)
		{
			it->second.full = false;
			readyPool.push(it->second);
			fullPool.erase(it->second);
		}
	}
}