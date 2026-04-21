#include "VulkanDescriptorSet.h"
#include "VulkanContext.h"
#include "VulkanShaderPass.h"
#include "VulkanBuffer.h"
#include "VulkanImageBuffer.h"
#include "VulkanDescriptorPool.h"

#include <mutex>
namespace sh::render::vk
{
	VulkanDescriptorSet::VulkanDescriptorSet() :
		context(nullptr),
		descSet(nullptr)
	{
	}
	VulkanDescriptorSet::VulkanDescriptorSet(VulkanDescriptorSet&& other) noexcept :
		context(other.context),
		descSet(other.descSet),
		set(other.set),
		descriptorTypes(std::move(other.descriptorTypes))
	{
		other.context = nullptr;
		other.descSet = nullptr;
	}
	VulkanDescriptorSet::~VulkanDescriptorSet()
	{
		Clear();
	}
	void VulkanDescriptorSet::Create(const IRenderContext& context, const ShaderPass& shader, UniformStructLayout::Usage usage)
	{
		Clear();

		this->context = &static_cast<const VulkanContext&>(context);
		const VulkanShaderPass& vkShader = static_cast<const VulkanShaderPass&>(shader);

		set = static_cast<uint32_t>(usage);

		if (auto opt = vkShader.GetSetLayout(set); opt.has_value())
		{
			const VulkanShaderPass::SetLayout& setLayout = opt.value();
			for (const auto& [binding, layoutBinding] : setLayout.descriptorSetLayoutBindings)
				descriptorTypes[binding] = layoutBinding.descriptorType;
		}

		const VkDescriptorSetLayout descLayout = vkShader.GetDescriptorSetLayout(set);
		if (descLayout != VK_NULL_HANDLE)
			descSet = this->context->GetDescriptorPool().AllocateDescriptorSet(descLayout);
	}
	void VulkanDescriptorSet::Clear()
	{
		if (descSet == nullptr || context == nullptr)
			return;

		context->GetDescriptorPool().FreeDescriptorSet(descSet);
		descSet = VK_NULL_HANDLE;
		descriptorTypes.clear();
	}
	void VulkanDescriptorSet::Link(uint32_t binding, const IBuffer& buffer, std::size_t bufferSize)
	{
		if (context == nullptr || descSet == nullptr)
			return;
		auto descriptorTypesIt = descriptorTypes.find(binding);
		if (descriptorTypesIt == descriptorTypes.end())
			return;

		const VulkanBuffer& vkBuffer = static_cast<const VulkanBuffer&>(buffer);
		
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = vkBuffer.GetBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = (bufferSize == 0) ? vkBuffer.GetSize() : bufferSize;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descSet;
		descriptorWrite.dstBinding = binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = descriptorTypesIt->second;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;
		descriptorWrite.pImageInfo = nullptr;
		descriptorWrite.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(context->GetDevice(), 1, &descriptorWrite, 0, nullptr);
	}
	void VulkanDescriptorSet::Link(uint32_t binding, const Texture& texture)
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
}//namespace