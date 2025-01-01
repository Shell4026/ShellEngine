#include "PCH.h"
#include "VulkanUniformBuffer.h"
#include "VulkanContext.h"
#include "VulkanShader.h"
#include "VulkanBuffer.h"
#include "VulkanTextureBuffer.h"
#include "VulkanDescriptorPool.h"

namespace sh::render::vk
{
	VulkanUniformBuffer::VulkanUniformBuffer() :
		context(nullptr),
		descSet(nullptr)
	{
	}
	VulkanUniformBuffer::VulkanUniformBuffer(VulkanUniformBuffer&& other) noexcept :
		context(other.context),
		descSet(other.descSet)
	{
		other.context = nullptr;
		other.descSet = nullptr;
	}
	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		Clean();
	}
	void VulkanUniformBuffer::Create(const IRenderContext& context, const Shader& shader, uint32_t type)
	{
		Clean();

		this->context = &static_cast<const VulkanContext&>(context);
		auto& vkShader = static_cast<const VulkanShader&>(shader);

		descSet = this->context->GetDescriptorPool().AllocateDescriptorSet(vkShader.GetDescriptorSetLayout(type), 1);
	}
	void VulkanUniformBuffer::Clean()
	{
		if (descSet == nullptr || context == nullptr)
			return;
		context->GetDescriptorPool().FreeDescriptorSet(descSet);
	}
	void VulkanUniformBuffer::Update(uint32_t binding, const IBuffer& buffer)
	{
		if (context == nullptr || descSet == nullptr)
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

		vkUpdateDescriptorSets(context->GetDevice(), 1, &descriptorWrite, 0, nullptr);
	}
	void VulkanUniformBuffer::Update(uint32_t binding, const Texture& texture)
	{
		if (context == nullptr || descSet == nullptr)
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

		vkUpdateDescriptorSets(context->GetDevice(), 1, &descriptorWrite, 0, nullptr);
	}
	auto VulkanUniformBuffer::GetVkDescriptorSet() const -> VkDescriptorSet
	{
		return descSet;
	}
}//namespace