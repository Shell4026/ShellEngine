#include "VulkanDrawable.h"

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
		mat(nullptr), mesh(nullptr), camera(nullptr),
		descriptorSet(),
		dirty(false)
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
		dirty(other.dirty)
	{
		other.mat = nullptr;
		other.mesh = nullptr;
		other.camera = nullptr;
	}

	VulkanDrawable::~VulkanDrawable() noexcept
	{
		Clean();
	}

	void VulkanDrawable::Clean()
	{
		vertUniformBuffers[GAME_THREAD].clear();
		vertUniformBuffers[RENDER_THREAD].clear();
		fragUniformBuffers[GAME_THREAD].clear();
		fragUniformBuffers[RENDER_THREAD].clear();
		textures.clear();

		pipeline.reset();
	}

	void VulkanDrawable::CreateDescriptorSet()
	{
		VulkanShader* shader = static_cast<VulkanShader*>(mat->GetShader());
		descriptorSet[GAME_THREAD] = renderer.GetDescriptorPool().AllocateDescriptorSet(shader->GetDescriptorSetLayout(), 1);
		descriptorSet[RENDER_THREAD] = renderer.GetDescriptorPool().AllocateDescriptorSet(shader->GetDescriptorSetLayout(), 1);
	}

	void VulkanDrawable::Build(Camera& camera, Mesh& mesh, Material* mat)
	{
		this->mat = mat;
		this->mesh = &mesh;
		this->camera = &camera;

		VulkanShader* shader = static_cast<VulkanShader*>(mat->GetShader());

		Clean();

		const impl::VulkanFramebuffer* vkFrameBuffer = nullptr;
		if (camera.GetRenderTexture() == nullptr)
			vkFrameBuffer = static_cast<const impl::VulkanFramebuffer*>(renderer.GetMainFramebuffer());
		else
			vkFrameBuffer = static_cast<const impl::VulkanFramebuffer*>(camera.GetRenderTexture()->GetFramebuffer());

		pipeline = std::make_unique<impl::VulkanPipeline>(renderer.GetDevice(), vkFrameBuffer->GetRenderPass());

		auto& bindings = static_cast<VulkanVertexBuffer*>(mesh.GetVertexBuffer())->bindingDescriptions;
		auto& attrs = static_cast<VulkanVertexBuffer*>(mesh.GetVertexBuffer())->attribDescriptions;
		
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
			auto data = shader->GetAttribute(mesh.attributes[i - 1]->name);
			if (!data)
				continue;
			if (data->typeName != mesh.attributes[i - 1]->typeName)
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

				vertUniformBuffers[i].insert({ uniform.first, std::move(buffer) });
			}
		}
		for (auto& uniform : shader->fragmentUniforms)
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

				fragUniformBuffers[i].insert({ uniform.first, std::move(buffer) });
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

		for (int threadIdx = 0; threadIdx < 2; ++threadIdx)
		{
			for (auto& buffer : vertUniformBuffers[threadIdx])
			{
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = buffer.second.GetBuffer();
				bufferInfo.offset = 0;
				bufferInfo.range = buffer.second.GetSize();

				VkWriteDescriptorSet descriptorWrite{};
				descriptorWrite.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = descriptorSet[threadIdx];
				descriptorWrite.dstBinding = buffer.first;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pBufferInfo = &bufferInfo;
				descriptorWrite.pImageInfo = nullptr;
				descriptorWrite.pTexelBufferView = nullptr;

				vkUpdateDescriptorSets(renderer.GetDevice(), 1, &descriptorWrite, 0, nullptr);
			}
			for (auto& buffer : fragUniformBuffers[threadIdx])
			{
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = buffer.second.GetBuffer();
				bufferInfo.offset = 0;
				bufferInfo.range = buffer.second.GetSize();

				VkWriteDescriptorSet descriptorWrite{};
				descriptorWrite.sType = VkStructureType::VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = descriptorSet[threadIdx];
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
				descriptorWrite.dstSet = descriptorSet[threadIdx];
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

	void VulkanDrawable::SetUniformData(uint32_t binding, const void* data, Stage stage)
	{
		if (stage == Stage::Vertex)
		{
			auto it = vertUniformBuffers[GAME_THREAD].find(binding);
			if (it == vertUniformBuffers[GAME_THREAD].end())
				return;

			it->second.SetData(data);
		}
		else if (stage == Stage::Fragment)
		{
			auto it = fragUniformBuffers[GAME_THREAD].find(binding);
			if (it == fragUniformBuffers[GAME_THREAD].end())
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
		descriptorWrite.dstSet = descriptorSet[GAME_THREAD];
		descriptorWrite.dstBinding = it->first;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = nullptr;
		descriptorWrite.pImageInfo = &imgInfo;
		descriptorWrite.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(renderer.GetDevice(), 1, &descriptorWrite, 0, nullptr);

		SetDirty();
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

	auto VulkanDrawable::GetPipeline() const -> impl::VulkanPipeline*
	{
		return pipeline.get();
	}
	auto VulkanDrawable::GetDescriptorSet() const -> VkDescriptorSet
	{
		return descriptorSet[RENDER_THREAD];
	}

	void VulkanDrawable::SetDirty()
	{
		if(!dirty)
			renderer.PushSyncObject(*this);
		dirty = true;
	}

	void VulkanDrawable::Sync()
	{
		if (!dirty)
			return;
		std::swap(descriptorSet[GAME_THREAD], descriptorSet[RENDER_THREAD]);
		std::swap(vertUniformBuffers[GAME_THREAD], vertUniformBuffers[RENDER_THREAD]);
		std::swap(fragUniformBuffers[GAME_THREAD], fragUniformBuffers[RENDER_THREAD]);

		dirty = false;
	}
}