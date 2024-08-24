﻿#include "VulkanDrawable.h"

#include "VulkanRenderer.h"
#include "Material.h"
#include "Mesh.h"
#include "VulkanShader.h"
#include "VulkanTextureBuffer.h"
#include "VulkanImpl/VulkanFramebuffer.h"
#include "VulkanImpl/VulkanDescriptorPool.h"

#include "Core/Reflection.hpp"

#include <cstring>
#include <utility>

namespace sh::render
{
	VulkanDrawable::VulkanDrawable(VulkanRenderer& renderer) :
		renderer(renderer), 
		mat(nullptr), mesh(nullptr),
		descriptorSet(), framebuffer(nullptr)
	{
	}
	VulkanDrawable::VulkanDrawable(VulkanDrawable&& other) noexcept :
		renderer(other.renderer),
		mat(other.mat), mesh(other.mesh),
		pipeline(std::move(other.pipeline)),
		descriptorSet(std::move(other.descriptorSet)),
		uniformBuffers(std::move(other.uniformBuffers)), textures(std::move(other.textures)),
		framebuffer(other.framebuffer)
	{
		other.mat = nullptr;
		other.mesh = nullptr;
		other.framebuffer = nullptr;
	}

	VulkanDrawable::~VulkanDrawable() noexcept
	{
		Clean();
	}

	void VulkanDrawable::Clean()
	{
		uniformBuffers[GAME_THREAD].clear();
		uniformBuffers[RENDER_THREAD].clear();

		pipeline.reset();
	}

	void VulkanDrawable::CreateDescriptorSet()
	{
		VulkanShader* shader = static_cast<VulkanShader*>(mat->GetShader());
		descriptorSet[GAME_THREAD] = renderer.GetDescriptorPool().AllocateDescriptorSet(shader->GetDescriptorSetLayout(), 1);
		descriptorSet[RENDER_THREAD] = renderer.GetDescriptorPool().AllocateDescriptorSet(shader->GetDescriptorSetLayout(), 1);
	}

	void VulkanDrawable::Build(Mesh* mesh, Material* mat)
	{
		this->mat = mat;
		this->mesh = mesh;

		VulkanShader* shader = static_cast<VulkanShader*>(mat->GetShader());

		Clean();

		const impl::VulkanFramebuffer* vkFrameBuffer = nullptr;
		if (framebuffer == nullptr)
			vkFrameBuffer = static_cast<const impl::VulkanFramebuffer*>(renderer.GetMainFramebuffer());
		else
			vkFrameBuffer = static_cast<const impl::VulkanFramebuffer*>(framebuffer);

		pipeline = std::make_unique<impl::VulkanPipeline>(renderer.GetDevice(), vkFrameBuffer->GetRenderPass());

		auto& bindings = static_cast<VulkanVertexBuffer*>(mesh->GetVertexBuffer())->bindingDescriptions;
		auto& attrs = static_cast<VulkanVertexBuffer*>(mesh->GetVertexBuffer())->attribDescriptions;
		
		//토폴리지
		impl::VulkanPipeline::Topology topology = impl::VulkanPipeline::Topology::Triangle;
		switch (shader->GetTopology())
		{
		case Shader::Topology::Point:
			topology = impl::VulkanPipeline::Topology::Point;
			break;
		case Shader::Topology::Line:
			topology = impl::VulkanPipeline::Topology::Line;
			break;
		}
		pipeline->SetTopology(topology);

		//Attribute
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

		//유니폼 버퍼, 바인딩 생성
		for (auto& uniform : shader->vertexUniforms)
		{
			size_t size = uniform.second.back().offset + uniform.second.back().size;

			for (int i = 0; i < 2; ++i)
			{
				impl::VulkanBuffer buffer{ renderer.GetDevice(), renderer.GetGPU(), renderer.GetAllocator() };
				auto result = buffer.Create(size,
					VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
					VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					true);
				assert(result == VkResult::VK_SUCCESS);

				uniformBuffers[i].insert({ uniform.first, std::move(buffer) });
			}
		}
		for (auto& uniform : shader->samplerFragmentUniforms)
		{
			Texture* tex = mat->GetTexture(uniform.second.name);
			if (tex != nullptr)
			{
				textures.insert({ uniform.first, tex });
			}
		}

		CreateDescriptorSet();

		for (int i = 0; i < 2; ++i)
		{
			for (auto& buffer : uniformBuffers[i])
			{
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = buffer.second.GetBuffer();
				bufferInfo.offset = 0;
				bufferInfo.range = buffer.second.GetSize();

				VkWriteDescriptorSet descriptorWrite{};
				descriptorWrite.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = descriptorSet[i];
				descriptorWrite.dstBinding = buffer.first;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pBufferInfo = &bufferInfo;
				descriptorWrite.pImageInfo = nullptr;
				descriptorWrite.pTexelBufferView = nullptr;

				vkUpdateDescriptorSets(renderer.GetDevice(), 1, &descriptorWrite, 0, nullptr);
			}
			for (auto& tex : textures)
			{
				VkDescriptorImageInfo  imgInfo{};
				imgInfo.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imgInfo.imageView = static_cast<VulkanTextureBuffer*>(tex.second->GetBuffer())->GetImageBuffer()->GetImageView();
				imgInfo.sampler = static_cast<VulkanTextureBuffer*>(tex.second->GetBuffer())->GetImageBuffer()->GetSampler();

				VkWriteDescriptorSet descriptorWrite{};
				descriptorWrite.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = descriptorSet[i];
				descriptorWrite.dstBinding = tex.first;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pBufferInfo = nullptr;
				descriptorWrite.pImageInfo = &imgInfo;
				descriptorWrite.pTexelBufferView = nullptr;

				vkUpdateDescriptorSets(renderer.GetDevice(), 1, &descriptorWrite, 0, nullptr);
			}
		}

		auto result = pipeline->
			SetShader(static_cast<VulkanShader*>(shader)).
			AddShaderStage(impl::VulkanPipeline::ShaderStage::Vertex).
			AddShaderStage(impl::VulkanPipeline::ShaderStage::Fragment).
			Build(shader->GetPipelineLayout());
		assert(result == VkResult::VK_SUCCESS);
	}

	void VulkanDrawable::SetUniformData(uint32_t binding, const void* data)
	{
		auto it = uniformBuffers[GAME_THREAD].find(binding);
		if (it == uniformBuffers[GAME_THREAD].end())
			return;
		
		it->second.SetData(data);
	}
	void VulkanDrawable::SetTextureData(uint32_t binding, Texture* tex)
	{
		textures[binding] = tex;
	}

	auto VulkanDrawable::GetMaterial() const -> Material*
	{
		return mat;
	}

	auto VulkanDrawable::GetMesh() const -> Mesh*
	{
		return mesh;
	}

	auto VulkanDrawable::GetPipeline() const -> impl::VulkanPipeline*
	{
		return pipeline.get();
	}
	auto VulkanDrawable::GetDescriptorSet() const -> VkDescriptorSet
	{
		return descriptorSet[RENDER_THREAD];
	}

	void VulkanDrawable::SetFramebuffer(Framebuffer& framebuffer)
	{
		this->framebuffer = &framebuffer;
	}
	auto VulkanDrawable::GetFramebuffer() const -> const Framebuffer*
	{
		return framebuffer;
	}

	void VulkanDrawable::SyncGameThread()
	{
		std::swap(descriptorSet[GAME_THREAD], descriptorSet[RENDER_THREAD]);
		std::swap(uniformBuffers[GAME_THREAD], uniformBuffers[RENDER_THREAD]);
	}
}