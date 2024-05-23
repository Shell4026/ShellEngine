#include "VulkanDrawable.h"

#include "VulkanRenderer.h"
#include "Material.h"
#include "Mesh.h"
#include "VulkanShader.h"
#include "VulkanImpl/VulkanFramebuffer.h"
#include "VulkanUniform.h"

#include "Core/Reflection.hpp"

#include <cstring>

namespace sh::render
{
	VulkanDrawable::VulkanDrawable(VulkanRenderer& renderer) :
		renderer(renderer), 
		cmd(renderer.GetDevice(), renderer.GetCommandPool()),
		pipelineLayout(nullptr), mat(nullptr), mesh(nullptr),
		descriptorSetLayout(nullptr)
	{
	}

	VulkanDrawable::~VulkanDrawable()
	{
		Clean();
	}

	void VulkanDrawable::Clean()
	{
		descriptorSets.clear();
		uniformBuffers.clear();
		cmd.Clean();
		pipeline.reset();
		if (pipelineLayout)
		{
			vkDestroyPipelineLayout(renderer.GetDevice(), pipelineLayout, nullptr);
			pipelineLayout = nullptr;
		}
		if (descriptorSetLayout)
		{
			vkDestroyDescriptorSetLayout(renderer.GetDevice(), descriptorSetLayout, nullptr);
			descriptorSetLayout = nullptr;
		}
	}

	auto VulkanDrawable::GetPipelineLayout() const -> VkPipelineLayout
	{
		return pipelineLayout;
	}

	auto VulkanDrawable::GetPipeline() const -> impl::VulkanPipeline*
	{
		return pipeline.get();
	}

	auto VulkanDrawable::CreatePipelineLayout() -> VkResult
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		return vkCreatePipelineLayout(renderer.GetDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout);
	}

	auto VulkanDrawable::CreateDescriptorLayout(uint32_t binding) -> VkResult
	{
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = binding;
		layoutBinding.descriptorCount = 1;
		layoutBinding.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBinding.pImmutableSamplers = nullptr;
		layoutBinding.stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutCreateInfo info{};
		info.sType = VkStructureType::VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.bindingCount = 1;
		info.pBindings = &layoutBinding;

		return vkCreateDescriptorSetLayout(renderer.GetDevice(), &info, nullptr, &descriptorSetLayout);
	}

	auto VulkanDrawable::CreateDescriptorSet() -> VkResult
	{
		//디스크립터 생성
		std::vector<VkDescriptorSetLayout> layouts(VulkanRenderer::MAX_FRAME_DRAW, descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = renderer.GetDescriptorPool();
		allocInfo.descriptorSetCount = static_cast<uint32_t>(VulkanRenderer::MAX_FRAME_DRAW);
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(VulkanRenderer::MAX_FRAME_DRAW);
		return vkAllocateDescriptorSets(renderer.GetDevice(), &allocInfo, descriptorSets.data());
	}

	void VulkanDrawable::Build(Mesh* mesh, Material* mat)
	{
		this->mat = mat;
		this->mesh = mesh;

		Shader* shader = mat->GetShader();

		Clean();

		auto frameBuffer = static_cast<const impl::VulkanFramebuffer*>(renderer.GetMainFramebuffer());
		pipeline = std::make_unique<impl::VulkanPipeline>(renderer.GetDevice(), frameBuffer->GetRenderPass());
		cmd.Create();

		auto& bindings = static_cast<VulkanVertexBuffer*>(mesh->GetVertexBuffer())->bindingDescriptions;
		auto& attrs = static_cast<VulkanVertexBuffer*>(mesh->GetVertexBuffer())->attribDescriptions;
		
		pipeline->AddBindingDescription(bindings[0]);
		pipeline->AddAttributeDescription(attrs[0]);
		for (int i = 1; i < attrs.size(); ++i)
		{
			auto data = shader->GetAttribute(mesh->attributes[i - 1]->name);
			if (!data)
				continue;
			if (data->typeName != mesh->attributes[i - 1]->typeName)
				continue;

			auto attrDesc = attrs[i];
			attrDesc.location = data->idx;

			pipeline->AddBindingDescription(bindings[i]);
			pipeline->AddAttributeDescription(attrDesc);
		}

		size_t size = mat->GetShader()->uniforms[0].back().offset + mat->GetShader()->uniforms[0].back().size;
		for (int i = 0; i < VulkanRenderer::MAX_FRAME_DRAW; ++i)
		{
			uniformBuffers.push_back(impl::VulkanBuffer{ renderer.GetDevice(), renderer.GetGPU(), renderer.GetAllocator() });
			auto result = uniformBuffers.back().Create(size,
				VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
				VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				true);
			assert(result == VkResult::VK_SUCCESS);
		}

		auto result = CreateDescriptorLayout(0);
		assert(result == VkResult::VK_SUCCESS);
		result = CreatePipelineLayout();
		assert(result == VkResult::VK_SUCCESS);

		result = CreateDescriptorSet();
		if (result == VkResult::VK_ERROR_OUT_OF_POOL_MEMORY)
		{
			renderer.ReAllocateDesriptorPool();
			return;
		}

		for (int i = 0; i < VulkanRenderer::MAX_FRAME_DRAW; ++i)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffers[i].GetBuffer();
			bufferInfo.offset = 0;
			bufferInfo.range =  size;

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr; // Optional
			descriptorWrite.pTexelBufferView = nullptr; // Optional

			vkUpdateDescriptorSets(renderer.GetDevice(), 1, &descriptorWrite, 0, nullptr);
		}

		pipeline->
			SetShader(static_cast<VulkanShader*>(shader)).
			AddShaderStage(impl::VulkanPipeline::ShaderStage::Vertex).
			AddShaderStage(impl::VulkanPipeline::ShaderStage::Fragment).
			Build(pipelineLayout);

		cmd.Clean();
	}

	void VulkanDrawable::SetUniformData(int frame, const void* data)
	{
		uniformBuffers[frame].SetData(data);
	}

	auto VulkanDrawable::GetMaterial() const -> Material*
	{
		return mat;
	}

	auto VulkanDrawable::GetMesh() const -> Mesh*
	{
		return mesh;
	}

	auto VulkanDrawable::GetDescriptorSet(int frame) -> VkDescriptorSet
	{
		return descriptorSets[frame];
	}
}