#pragma once

#include "Render/Export.h"
#include "VulkanConfig.h"

#include "Core/NonCopyable.h"
#include "Core/SContainer.hpp"

#include <stack>
#include <unordered_map>

namespace sh::render::vk
{
	class VulkanDescriptorPool : core::INonCopyable
	{
	private:
		VkDevice device;

		struct Pool
		{
			VkDescriptorPool pool;
			bool full;

			bool operator==(const Pool& other) const
			{
				return pool == other.pool;
			}
		};

		struct PoolHasher
		{
			std::size_t operator()(const Pool& p) const noexcept
			{
				std::size_t h1 = std::hash<VkDescriptorPool>{}(p.pool);
				std::size_t h2 = std::hash<bool>{}(p.full);
				return h1 ^ (h2 << 1);
			}
		};

	private:
		size_t initialSize;
		size_t size;

		core::SHashSet<Pool, 32, PoolHasher> fullPool;
		std::stack<Pool> readyPool;

		core::SHashMap<VkDescriptorSet, Pool> allocated;
	private:
		auto GetPool() -> Pool&;
	public:
		SH_RENDER_API VulkanDescriptorPool(VkDevice device, size_t size = 128);
		SH_RENDER_API VulkanDescriptorPool(VulkanDescriptorPool&& other) noexcept;
		SH_RENDER_API ~VulkanDescriptorPool();

		SH_RENDER_API auto AllocateDescriptorSet(VkDescriptorSetLayout layouts, uint32_t count) -> VkDescriptorSet;
		SH_RENDER_API void FreeDescriptorSet(VkDescriptorSet descSet);

		SH_RENDER_API void Clean();
	};
}