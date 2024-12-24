#include "PCH.h"
#include "VulkanUniformBuffer.h"
#include "VulkanDescriptorPool.h"
#include "VulkanRenderer.h"
#include "VulkanShader.h"
#include "VulkanBuffer.h"
#include "VulkanTextureBuffer.h"

#include "Core/Logger.h"

#include <algorithm>

namespace sh::render::vk
{
	VulkanUniformBuffer::VulkanUniformBuffer() :
		renderer(nullptr),
		descSet(nullptr)
	{
	}
	VulkanUniformBuffer::VulkanUniformBuffer(VulkanUniformBuffer&& other) noexcept :
		renderer(other.renderer),
		descSet(other.descSet)
	{
		other.renderer = nullptr;
		other.descSet = nullptr;
	}
	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		Clean();
	}
	void VulkanUniformBuffer::Create(const Renderer& renderer, const Shader& shader, uint32_t type)
	{
		Clean();

		auto& vkRenderer = static_cast<const VulkanRenderer&>(renderer);
		this->renderer = &vkRenderer;
		auto& vkShader = static_cast<const VulkanShader&>(shader);

		descSet = vkRenderer.GetDescriptorPool().AllocateDescriptorSet(vkShader.GetDescriptorSetLayout(type), 1);
	}
	void VulkanUniformBuffer::Clean()
	{
		if (descSet && renderer)
		{
			renderer->GetDescriptorPool().FreeDescriptorSet(descSet);
		}
	}
	void VulkanUniformBuffer::Update(uint32_t binding, const IBuffer& buffer)
	{
		if (renderer == nullptr || descSet == nullptr)
			return;

		auto& vkBuffer = static_cast<const VulkanBuffer&>(buffer);
		
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = vkBuffer.GetBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = vkBuffer.GetSize();

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descSet;
		descriptorWrite.dstBinding = binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;
		descriptorWrite.pImageInfo = nullptr;
		descriptorWrite.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(renderer->GetDevice(), 1, &descriptorWrite, 0, nullptr);
	}
	void VulkanUniformBuffer::Update(uint32_t binding, const Texture& texture)
	{
		if (renderer == nullptr || descSet == nullptr)
			return;

		VkDescriptorImageInfo imgInfo{};
		imgInfo.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imgInfo.imageView = static_cast<VulkanTextureBuffer*>(texture.GetBuffer())->GetImageBuffer()->GetImageView();
		imgInfo.sampler = static_cast<VulkanTextureBuffer*>(texture.GetBuffer())->GetImageBuffer()->GetSampler();

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descSet;
		descriptorWrite.dstBinding = binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = nullptr;
		descriptorWrite.pImageInfo = &imgInfo;
		descriptorWrite.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(renderer->GetDevice(), 1, &descriptorWrite, 0, nullptr);
	}
	auto VulkanUniformBuffer::GetVkDescriptorSet() const -> VkDescriptorSet
	{
		return descSet;
	}
}//namespace