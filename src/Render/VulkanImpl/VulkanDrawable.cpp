#include "pch.h"
#include "VulkanDrawable.h"

#include "Material.h"
#include "Mesh.h"

#include "VulkanRenderer.h"
#include "VulkanShader.h"
#include "VulkanTextureBuffer.h"
#include "VulkanFramebuffer.h"
#include "VulkanDescriptorPool.h"
#include "VulkanVertexBuffer.h"

#include "Core/Reflection.hpp"

#include <cstring>
#include <utility>

namespace sh::render
{
	VulkanDrawable::VulkanDrawable(VulkanRenderer& renderer) :
		renderer(renderer), 
		mat(nullptr), mesh(nullptr), camera(nullptr),
		descriptorSet(),
		bInit(false), bDirty(false), bTextureDirty(false), bPipelineDirty(false)
	{
	}
	VulkanDrawable::VulkanDrawable(VulkanDrawable&& other) noexcept :
		renderer(other.renderer),
		mat(other.mat), mesh(other.mesh), camera(other.camera),
		pipeline(std::move(other.pipeline)),
		descriptorSet(std::move(other.descriptorSet)),
		vertUniformBuffers(std::move(other.vertUniformBuffers)), 
		fragUniformBuffers(std::move(other.fragUniformBuffers)),
		textures(std::move(other.textures)),
		bInit(other.bInit), bDirty(other.bDirty), bTextureDirty(other.bTextureDirty), bPipelineDirty(other.bPipelineDirty)
	{
		other.mat = nullptr;
		other.mesh = nullptr;
		other.camera = nullptr;

		other.bInit = false;
		other.bDirty = false;
		other.bTextureDirty = false;
		other.bPipelineDirty = false;
	}

	VulkanDrawable::~VulkanDrawable() noexcept
	{
		Clean();
	}

	void VulkanDrawable::Clean()
	{
		vertUniformBuffers[core::ThreadType::Game].clear();
		fragUniformBuffers[core::ThreadType::Game].clear();
		textures.clear();
		vertUniformBuffers[core::ThreadType::Render].clear();
		fragUniformBuffers[core::ThreadType::Render].clear();
		pipeline[core::ThreadType::Game].reset();
		pipeline[core::ThreadType::Render].reset();
	}

	void VulkanDrawable::CreateUniformBuffers(core::ThreadType thr, const Shader& shader)
	{
		for (auto& [binding, data] : shader.vertexUniforms)
		{
			size_t size = data.back().offset + data.back().size;

			impl::VulkanBuffer buffer{ renderer.GetDevice(), renderer.GetGPU(), renderer.GetAllocator() };
			auto result = buffer.Create(size,
				VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
				VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				true);
			assert(result == VkResult::VK_SUCCESS);

			vertUniformBuffers[thr].insert({ binding, std::move(buffer) });
		}
		for (auto& [binding, data] : shader.fragmentUniforms)
		{
			size_t size = data.back().offset + data.back().size;

			impl::VulkanBuffer buffer{ renderer.GetDevice(), renderer.GetGPU(), renderer.GetAllocator() };
			auto result = buffer.Create(size,
				VkBufferUsageFlagBits::VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
				VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				true);
			assert(result == VkResult::VK_SUCCESS);

			fragUniformBuffers[thr].insert({ binding, std::move(buffer) });
		}
		for (auto& [binding, data] : shader.samplerFragmentUniforms)
		{
			Texture* tex = mat->GetTexture(data.name);
			if (tex != nullptr)
			{
				textures.insert({ binding, tex });
			}
		}
	}

	void VulkanDrawable::UpdateDescriptors(core::ThreadType thr)
	{
		// 디스크립터셋 데이터 업데이트
		for (auto& [binding, buffer] : vertUniformBuffers[thr])
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = buffer.GetBuffer();
			bufferInfo.offset = 0;
			bufferInfo.range = buffer.GetSize();

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSet[thr];
			descriptorWrite.dstBinding = binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr;
			descriptorWrite.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(renderer.GetDevice(), 1, &descriptorWrite, 0, nullptr);
		}
		for (auto& [binding, buffer] : fragUniformBuffers[thr])
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = buffer.GetBuffer();
			bufferInfo.offset = 0;
			bufferInfo.range = buffer.GetSize();

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSet[thr];
			descriptorWrite.dstBinding = binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr;
			descriptorWrite.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(renderer.GetDevice(), 1, &descriptorWrite, 0, nullptr);
		}
		for (auto& [binding, buffer] : textures)
		{
			VkDescriptorImageInfo  imgInfo{};
			imgInfo.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imgInfo.imageView = static_cast<VulkanTextureBuffer*>(buffer->GetBuffer())->GetImageBuffer()->GetImageView();
			imgInfo.sampler = static_cast<VulkanTextureBuffer*>(buffer->GetBuffer())->GetImageBuffer()->GetSampler();

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSet[thr];
			descriptorWrite.dstBinding = binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = nullptr;
			descriptorWrite.pImageInfo = &imgInfo;
			descriptorWrite.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(renderer.GetDevice(), 1, &descriptorWrite, 0, nullptr);
		}
	}

	void VulkanDrawable::Build(Camera& camera, Mesh& mesh, Material* mat)
	{
		this->mat = mat;
		this->mesh = &mesh;
		this->camera = &camera;

		VulkanShader* shader = static_cast<VulkanShader*>(mat->GetShader());
		assert(shader);

		const impl::VulkanFramebuffer* vkFrameBuffer = nullptr;
		if (camera.GetRenderTexture() == nullptr)
			vkFrameBuffer = static_cast<const impl::VulkanFramebuffer*>(renderer.GetMainFramebuffer());
		else
			vkFrameBuffer = static_cast<const impl::VulkanFramebuffer*>(camera.GetRenderTexture()->GetFramebuffer());

		if (!bInit)
		{
			for (int thr = 0; thr < pipeline.size(); ++thr)
			{
				core::ThreadType type = static_cast<core::ThreadType>(thr);

				pipeline[type] = std::make_unique<impl::VulkanPipeline>(renderer.GetDevice(), vkFrameBuffer->GetRenderPass());

				CreateUniformBuffers(type, *shader);

				descriptorSet[type] = renderer.GetDescriptorPool().AllocateDescriptorSet(shader->GetDescriptorSetLayout(), 1);

				UpdateDescriptors(type);
			}
			bInit = true;
		}
		else
		{
			core::ThreadType type = core::ThreadType::Game;

			pipeline[type] = std::make_unique<impl::VulkanPipeline>(renderer.GetDevice(), vkFrameBuffer->GetRenderPass());
			
			CreateUniformBuffers(type, *shader);

			renderer.GetDescriptorPool().FreeDescriptorSet(descriptorSet[type]);
			descriptorSet[type] = renderer.GetDescriptorPool().AllocateDescriptorSet(shader->GetDescriptorSetLayout(), 1);

			UpdateDescriptors(type);
		}
		
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
		pipeline[core::ThreadType::Game]->SetTopology(topology);

		//Attribute
		auto& bindings = static_cast<VulkanVertexBuffer*>(mesh.GetVertexBuffer())->bindingDescriptions;
		auto& attrs = static_cast<VulkanVertexBuffer*>(mesh.GetVertexBuffer())->attribDescriptions;

		// binding 0과 attribute 0은 버텍스
		for (int i = 0; i < attrs.size(); ++i)
		{
			auto data = shader->GetAttribute(mesh.attributes[i]->name);
			if (!data)
				continue;
			if (data->typeName != mesh.attributes[i]->typeName)
				continue;

			VkVertexInputAttributeDescription attrDesc = attrs[i];
			attrDesc.location = data->idx;

			pipeline[core::ThreadType::Game]->AddBindingDescription(bindings[i]);
			pipeline[core::ThreadType::Game]->AddAttributeDescription(attrDesc);
		}

		auto result = pipeline[core::ThreadType::Game]->
			SetShader(static_cast<VulkanShader*>(shader)).
			AddShaderStage(impl::VulkanPipeline::ShaderStage::Vertex).
			AddShaderStage(impl::VulkanPipeline::ShaderStage::Fragment).
			Build(shader->GetPipelineLayout());

		bPipelineDirty = true;
		SetDirty();

		assert(result == VkResult::VK_SUCCESS);
	}

	void VulkanDrawable::SetUniformData(uint32_t binding, const void* data, Stage stage)
	{
		if (stage == Stage::Vertex)
		{
			auto it = vertUniformBuffers[core::ThreadType::Game].find(binding);
			if (it == vertUniformBuffers[core::ThreadType::Game].end())
				return;

			it->second.SetData(data);
		}
		else if (stage == Stage::Fragment)
		{
			auto it = fragUniformBuffers[core::ThreadType::Game].find(binding);
			if (it == fragUniformBuffers[core::ThreadType::Game].end())
				return;

			it->second.SetData(data);
		}

		SetDirty();
	}
	void VulkanDrawable::SetTextureData(uint32_t binding, Texture* tex)
	{
		auto it = textures.find(binding);
		if (it == textures.end())
			return;
		if (it->second == tex)
			return;

		it->second = tex;

		VkDescriptorImageInfo  imgInfo{};
		imgInfo.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imgInfo.imageView = static_cast<VulkanTextureBuffer*>(it->second->GetBuffer())->GetImageBuffer()->GetImageView();
		imgInfo.sampler = static_cast<VulkanTextureBuffer*>(it->second->GetBuffer())->GetImageBuffer()->GetSampler();

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet[core::ThreadType::Game];
		descriptorWrite.dstBinding = it->first;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = nullptr;
		descriptorWrite.pImageInfo = &imgInfo;
		descriptorWrite.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(renderer.GetDevice(), 1, &descriptorWrite, 0, nullptr);

		SetDirty();
		bTextureDirty = true;
	}

	auto VulkanDrawable::GetMaterial() const -> Material*
	{
		return mat;
	}
	auto VulkanDrawable::GetMesh() const -> Mesh*
	{
		return mesh;
	}
	auto VulkanDrawable::GetCamera() const -> Camera*
	{
		return camera;
	}

	auto VulkanDrawable::GetPipeline(core::ThreadType thr) const -> impl::VulkanPipeline*
	{
		return pipeline[static_cast<int>(thr)].get();
	}
	auto VulkanDrawable::GetDescriptorSet() const -> VkDescriptorSet
	{
		return descriptorSet[core::ThreadType::Render];
	}

	void VulkanDrawable::SetDirty()
	{
		if (bDirty)
			return;

		renderer.GetThreadSyncManager().PushSyncable(*this);
		bDirty = true;
	}

	void VulkanDrawable::Sync()
	{
		if (!bDirty)
			return;

		if (bPipelineDirty)
			std::swap(pipeline[core::ThreadType::Game], pipeline[core::ThreadType::Render]);
		if (bTextureDirty)
			std::swap(descriptorSet[core::ThreadType::Game], descriptorSet[core::ThreadType::Render]);
		std::swap(vertUniformBuffers[core::ThreadType::Game], vertUniformBuffers[core::ThreadType::Render]);
		std::swap(fragUniformBuffers[core::ThreadType::Game], fragUniformBuffers[core::ThreadType::Render]);

		bDirty = false;
		bTextureDirty = false;
		bPipelineDirty = false;
	}
}