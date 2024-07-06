#include "VulkanDescriptorPool.h"

#include <array>
#include <cassert>
namespace sh::render::impl
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
	}
	VulkanDescriptorPool::~VulkanDescriptorPool()
	{
		Clean();
	}

	auto VulkanDescriptorPool::GetPool() -> VkDescriptorPool
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
			readyPool.push_back(descPool);

			size *= 1.5f;
			if (size > 4096) 
				size = 4096;

			return readyPool.back();
		}
		return readyPool.back();
	}

	auto VulkanDescriptorPool::AllocateDescriptorSet(VkDescriptorSetLayout layouts, uint32_t count) -> VkDescriptorSet
	{
		VkDescriptorPool pool = GetPool();
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = pool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &layouts;

		VkDescriptorSet descriptorSet;
		auto result = vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);
		if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL)
		{
			readyPool.pop_back();
			fullPool.push_back(pool);
			pool = GetPool();
			allocInfo.descriptorPool = pool;
			result = vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);
			assert(result == VkResult::VK_SUCCESS);
		}

		allocated.insert({ descriptorSet, pool });

		return descriptorSet;
	}

	void VulkanDescriptorPool::Clean()
	{
		allocated.clear();
		size = initialSize;
		for (auto pool : fullPool)
			vkDestroyDescriptorPool(device, pool, nullptr);
		fullPool.clear();

		for (auto pool : readyPool)
			vkDestroyDescriptorPool(device, pool, nullptr);
		readyPool.clear();
	}
	void VulkanDescriptorPool::FreeDescriptorSet(VkDescriptorSet descSet)
	{
		auto it = allocated.find(descSet);
		if (it == allocated.end())
			return;

		vkFreeDescriptorSets(device, it->second, 1, &it->first);
	}
}