#include "VulkanUniform.h"

#include "VulkanRenderer.h"
#include "Material.h"

#include <cassert>

namespace sh::render
{
	VulkanUniform::VulkanUniform(const VulkanRenderer& renderer) :
		renderer(renderer), 
		descriptorSetLayout(nullptr)
	{
	}

	VulkanUniform::VulkanUniform(VulkanUniform&& other) noexcept :
		renderer(other.renderer),
		descriptorSets(std::move(other.descriptorSets)), descriptorSetLayout(other.descriptorSetLayout),
		uniformBuffers(std::move(other.uniformBuffers))
	{
		other.descriptorSetLayout = nullptr;
	}

	VulkanUniform::~VulkanUniform()
	{
		Clean();
	}

	void VulkanUniform::Clean()
	{
		for (auto& buffer : uniformBuffers)
		{
			buffer.Clean();
		}

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
		allocInfo.descriptorSetCount = static_cast<uint32_t>(VulkanRenderer::MAX_FRAME_DRAW);
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(VulkanRenderer::MAX_FRAME_DRAW);
		auto result = vkAllocateDescriptorSets(renderer.GetDevice(), &allocInfo, descriptorSets.data());
		assert(result == VK_SUCCESS);
		return result;


		for (int i = 0; i < VulkanRenderer::MAX_FRAME_DRAW; ++i)
		{
			uniformBuffers.push_back(impl::VulkanBuffer{ renderer.GetDevice(), renderer.GetGPU() });
			result = uniformBuffers.back().Create(dataSize,
				VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
				VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				true);
			assert(result == VkResult::VK_SUCCESS);
			if (result != VkResult::VK_SUCCESS)
				return result;

			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffers[i].GetBuffer();
			bufferInfo.offset = 0;
			//bufferInfo.range = sizeof(UniformBufferObject);
		}
	}
}