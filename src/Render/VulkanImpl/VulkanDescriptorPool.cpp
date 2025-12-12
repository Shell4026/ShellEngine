#include "VulkanDescriptorPool.h"

#include <array>
#include <stdexcept>
#include <cassert>

namespace sh::render::vk
{
	VulkanDescriptorPool::VulkanDescriptorPool(VkDevice device, uint32_t initialSets, uint32_t maxSets) :
		device(device),
		initialSize(initialSets),
		nextSize(initialSets),
		maxSize(maxSets)
	{
		assert(device != VK_NULL_HANDLE);
		assert(initialSize > 0);
		assert(maxSize >= initialSize);
	}
	VulkanDescriptorPool::VulkanDescriptorPool(VulkanDescriptorPool&& other) noexcept :
		device(other.device),
		initialSize(other.initialSize), 
		nextSize(other.nextSize),
		maxSize(other.maxSize),
		readyPools(std::move(other.readyPools)),
		fullPools(std::move(other.fullPools)),
		setToPool(std::move(other.setToPool))
	{
		other.device = VK_NULL_HANDLE;
		other.initialSize = 0;
		other.nextSize = 0;
		other.maxSize = 0;
	}
	VulkanDescriptorPool::~VulkanDescriptorPool()
	{
		Clear();
	}
	SH_RENDER_API auto VulkanDescriptorPool::operator=(VulkanDescriptorPool&& other) noexcept -> VulkanDescriptorPool&
	{
		if (this == &other) 
			return *this;

		Clear();

		device = other.device;
		initialSize = other.initialSize;
		nextSize = other.nextSize;
		maxSize = other.maxSize;

		readyPools = std::move(other.readyPools);
		fullPools = std::move(other.fullPools);
		setToPool = std::move(other.setToPool);

		other.device = VK_NULL_HANDLE;
		other.initialSize = other.nextSize = other.maxSize = 0;

		return *this;
	}
	SH_RENDER_API auto VulkanDescriptorPool::AllocateDescriptorSet(VkDescriptorSetLayout layout) -> VkDescriptorSet
	{
		auto sets = AllocateDescriptorSets(layout, 1);

		return sets[0];
	}
	SH_RENDER_API auto VulkanDescriptorPool::AllocateDescriptorSets(VkDescriptorSetLayout layout, uint32_t count) -> std::vector<VkDescriptorSet>
	{
		if (count == 0)
			return {};

		std::vector<VkDescriptorSetLayout> layouts(count, layout);
		std::vector<VkDescriptorSet> sets(count, VK_NULL_HANDLE);

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorSetCount = count;
		allocInfo.pSetLayouts = layouts.data();

		while (true)
		{
			VkDescriptorPool pool = AcquirePool();
			allocInfo.descriptorPool = pool;

			VkResult result = vkAllocateDescriptorSets(device, &allocInfo, sets.data());
			if (result == VkResult::VK_SUCCESS)
			{
				for (auto s : sets)
					setToPool.emplace(s, pool);
				return sets;
			}

			if (result == VkResult::VK_ERROR_OUT_OF_POOL_MEMORY || result == VkResult::VK_ERROR_FRAGMENTED_POOL)
			{
				// 현재 top 풀에서 실패했으니 full로 마킹 후 다음 풀로 시도
				if (!readyPools.empty() && readyPools.top() == pool)
					readyPools.pop();
				fullPools.insert(pool);

				// 다음 풀에서 반복
				continue;
			}

			throw std::runtime_error("vkAllocateDescriptorSets failed (unexpected error).");
		}
	}
	SH_RENDER_API void VulkanDescriptorPool::FreeDescriptorSet(VkDescriptorSet descSet)
	{
		if (descSet == VK_NULL_HANDLE || device == VK_NULL_HANDLE)
			return;

		auto it = setToPool.find(descSet);
		if (it == setToPool.end())
			return;

		VkDescriptorPool pool = it->second;

		vkFreeDescriptorSets(device, pool, 1, &descSet);
		setToPool.erase(it);

		auto fit = fullPools.find(pool);
		if (fit != fullPools.end())
		{
			fullPools.erase(fit);
			readyPools.push(pool);
		}
	}
	SH_RENDER_API void VulkanDescriptorPool::Clear()
	{
		if (device == VK_NULL_HANDLE)
		{
			readyPools = {};
			fullPools.clear();
			setToPool.clear();
			initialSize = nextSize = maxSize = 0;
			return;
		}

		vkDeviceWaitIdle(device);

		setToPool.clear();
		DestroyAllPools();

		nextSize = initialSize;
	}
	auto VulkanDescriptorPool::AcquirePool() -> VkDescriptorPool
	{
		if (!readyPools.empty())
			return readyPools.top();

		VkDescriptorPool pool = CreatePool(nextSize);
		readyPools.push(pool);

		uint32_t grown = static_cast<uint32_t>(static_cast<float>(nextSize) * 1.5f);
		if (grown < nextSize) 
			grown = nextSize;
		nextSize = (grown > maxSize) ? maxSize : grown;

		return pool;
	}
	auto VulkanDescriptorPool::CreatePool(uint32_t setCapacity) -> VkDescriptorPool
	{
		std::array<VkDescriptorPoolSize, 3> poolSizes{};
		poolSizes[0] = { VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, setCapacity };
		poolSizes[1] = { VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, setCapacity };
		poolSizes[2] = { VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, setCapacity };

		VkDescriptorPoolCreateInfo info{};
		info.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		info.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		info.pPoolSizes = poolSizes.data();
		info.maxSets = setCapacity;
		info.flags = VkDescriptorPoolCreateFlagBits::VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

		VkDescriptorPool pool = VK_NULL_HANDLE;
		VkResult result = vkCreateDescriptorPool(device, &info, nullptr, &pool);
		if (result != VkResult::VK_SUCCESS)
			throw std::runtime_error("vkCreateDescriptorPool failed.");

		return pool;
	}
	void VulkanDescriptorPool::DestroyAllPools()
	{
		while (!readyPools.empty())
		{
			VkDescriptorPool p = readyPools.top();
			readyPools.pop();
			if (p != VK_NULL_HANDLE)
				vkDestroyDescriptorPool(device, p, nullptr);
		}

		// full 파괴
		for (auto p : fullPools)
		{
			if (p != VK_NULL_HANDLE)
				vkDestroyDescriptorPool(device, p, nullptr);
		}
		fullPools.clear();
	}
}