#include "VulkanUniformBuffer.h"
#include "VulkanContext.h"
#include "VulkanShaderPass.h"
#include "VulkanBuffer.h"
#include "VulkanImageBuffer.h"
#include "VulkanDescriptorPool.h"

#include <mutex>
namespace sh::render::vk
{
	VulkanUniformBuffer::VulkanUniformBuffer() :
		context(nullptr),
		descSet(nullptr)
	{
	}
	VulkanUniformBuffer::VulkanUniformBuffer(VulkanUniformBuffer&& other) noexcept :
		context(other.context),
		descSet(other.descSet),
		bDynamic(other.bDynamic)
	{
		other.context = nullptr;
		other.descSet = nullptr;
	}
	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		Clear();
	}
	void VulkanUniformBuffer::Create(const IRenderContext& context, const ShaderPass& shader, UniformStructLayout::Type type)
	{
		Clear();

		this->context = &static_cast<const VulkanContext&>(context);
		auto& vkShader = static_cast<const VulkanShaderPass&>(shader);

		set = static_cast<uint32_t>(type);

		auto layoutInfo = vkShader.GetDescriptorSetLayout(set);
		VkDescriptorSetLayout layout = layoutInfo.layout;
		bDynamic = layoutInfo.bDynamic;

		if (layout != VK_NULL_HANDLE)
			descSet = this->context->GetDescriptorPool().AllocateDescriptorSet(layout);
	}
	void VulkanUniformBuffer::Clear()
	{
		if (descSet == nullptr || context == nullptr)
			return;

		context->GetDescriptorPool().FreeDescriptorSet(descSet);
	}
	void VulkanUniformBuffer::Link(uint32_t binding, const IBuffer& buffer, std::size_t bufferSize)
	{
		if (context == nullptr || descSet == nullptr)
			return;

		auto& vkBuffer = static_cast<const VulkanBuffer&>(buffer);
		
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = vkBuffer.GetBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = (bufferSize == 0) ? vkBuffer.GetSize() : bufferSize;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descSet;
		descriptorWrite.dstBinding = binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = bDynamic ? 
			VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;
		descriptorWrite.pImageInfo = nullptr;
		descriptorWrite.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(context->GetDevice(), 1, &descriptorWrite, 0, nullptr);
	}
	void VulkanUniformBuffer::Link(uint32_t binding, const Texture& texture)
	{
		if (context == nullptr || descSet == nullptr)
			return;

		VkDescriptorImageInfo imgInfo{};
		imgInfo.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imgInfo.imageView = static_cast<VulkanImageBuffer*>(texture.GetTextureBuffer())->GetImageView();
		imgInfo.sampler = static_cast<VulkanImageBuffer*>(texture.GetTextureBuffer())->GetSampler();

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
	SH_RENDER_API auto VulkanUniformBuffer::GetSetNumber() const -> uint32_t
	{
		return set;
	}
	SH_RENDER_API auto VulkanUniformBuffer::GetVkDescriptorSet() const -> VkDescriptorSet
	{
		return descSet;
	}
}//namespace