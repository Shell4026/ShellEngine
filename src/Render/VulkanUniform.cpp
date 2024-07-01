#include "VulkanUniform.h"

#include "VulkanRenderer.h"
#include "Material.h"

#include <cassert>

namespace sh::render
{
	VulkanUniform::VulkanUniform(const VulkanRenderer& renderer) :
		renderer(renderer), 
		descriptorSetLayout(nullptr),
		uniformBuffer(renderer.GetDevice(), renderer.GetGPU(), renderer.GetAllocator())
	{
	}

	VulkanUniform::VulkanUniform(VulkanUniform&& other) noexcept :
		renderer(other.renderer),
		descriptorSet(other.descriptorSet), descriptorSetLayout(other.descriptorSetLayout),
		uniformBuffer(std::move(other.uniformBuffer))
	{
		other.descriptorSetLayout = nullptr;
		other.descriptorSet = nullptr;
	}

	VulkanUniform::~VulkanUniform()
	{
		Clean();
	}

	void VulkanUniform::Clean()
	{
		uniformBuffer.Clean();

		if (descriptorSetLayout)
		{
			vkDestroyDescriptorSetLayout(renderer.GetDevice(), descriptorSetLayout, nullptr);
			descriptorSetLayout = nullptr;
		}
	}



	auto VulkanUniform::Create(uint32_t binding, size_t dataSize) -> VkResult
	{
		
		std::vector<VkDescriptorSetLayout> layouts{ 2, descriptorSetLayout };
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = renderer.GetDescriptorPool();
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = layouts.data();
;
		auto result = vkAllocateDescriptorSets(renderer.GetDevice(), &allocInfo, &descriptorSet);
		assert(result == VK_SUCCESS);
		return result;


		result = uniformBuffer.Create(dataSize,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			true);
		assert(result == VkResult::VK_SUCCESS);
		if (result != VkResult::VK_SUCCESS)
			return result;

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = uniformBuffer.GetBuffer();
		bufferInfo.offset = 0;
		//bufferInfo.range = sizeof(UniformBufferObject);
	}
}