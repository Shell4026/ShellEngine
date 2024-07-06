#pragma once

#include "Render/Export.h"
#include "VulkanConfig.h"

#include "Core/NonCopyable.h"

#include <vector>
#include <unordered_map>

namespace sh::render::impl
{
	class VulkanDescriptorPool : core::INonCopyable
	{
	private:
		VkDevice device;
	private:
		size_t initialSize;
		size_t size;

		std::vector<VkDescriptorPool> fullPool;
		std::vector<VkDescriptorPool> readyPool;

		std::unordered_map<VkDescriptorSet, VkDescriptorPool> allocated;
	private:
		auto GetPool() -> VkDescriptorPool;
	public:
		SH_RENDER_API VulkanDescriptorPool(VkDevice device, size_t size = 16);
		SH_RENDER_API VulkanDescriptorPool(VulkanDescriptorPool&& other) noexcept;
		SH_RENDER_API ~VulkanDescriptorPool();

		SH_RENDER_API auto AllocateDescriptorSet(VkDescriptorSetLayout layouts, uint32_t count) -> VkDescriptorSet;
		SH_RENDER_API void FreeDescriptorSet(VkDescriptorSet descSet);

		SH_RENDER_API void Clean();
	};
}