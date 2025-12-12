#pragma once
#include "Render/Export.h"
#include "VulkanConfig.h"

#include "Core/NonCopyable.h"

#include <stack>
#include <unordered_map>
#include <unordered_set>

namespace sh::render::vk
{
	/// @brief 디스크립터 풀 추상화 클래스.
	/// @brief 멀티스레드라면 스레드 별로 생성해서 쓸 것
	class VulkanDescriptorPool : core::INonCopyable
	{
	public:
		/// @param initialSets 첫 풀의 maxSets 및 각 타입 descriptorCount 기준
		/// @param maxSets 풀 크기 증가 상한
		SH_RENDER_API VulkanDescriptorPool(VkDevice device, uint32_t initialSets = 128, uint32_t maxSets = 4096);
		SH_RENDER_API VulkanDescriptorPool(VulkanDescriptorPool&& other) noexcept;
		SH_RENDER_API ~VulkanDescriptorPool();

		SH_RENDER_API auto operator=(VulkanDescriptorPool&& other) noexcept -> VulkanDescriptorPool&;

		SH_RENDER_API auto AllocateDescriptorSet(VkDescriptorSetLayout layout) -> VkDescriptorSet;
		SH_RENDER_API auto AllocateDescriptorSets(VkDescriptorSetLayout layout, uint32_t count) -> std::vector<VkDescriptorSet>;
		SH_RENDER_API void FreeDescriptorSet(VkDescriptorSet descSet);

		/// @brief 내부 풀 전부 destroy (set들은 무효화됨). size는 initial로 리셋
		SH_RENDER_API void Clear();
	private:
		auto AcquirePool() -> VkDescriptorPool;
		auto CreatePool(uint32_t setCapacity) -> VkDescriptorPool;
		void DestroyAllPools();
	private:
		VkDevice device;

		uint32_t initialSize = 0;
		uint32_t nextSize = 0;
		uint32_t maxSize = 0;

		std::stack<VkDescriptorPool> readyPools;
		std::unordered_set<VkDescriptorPool> fullPools;

		std::unordered_map<VkDescriptorSet, VkDescriptorPool> setToPool;
	};
}