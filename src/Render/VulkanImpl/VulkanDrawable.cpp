#include "pch.h"
#include "VulkanDrawable.h"

#include "Material.h"
#include "Mesh.h"
#include "BufferFactory.h"

#include "VulkanRenderer.h"
#include "VulkanShader.h"
#include "VulkanTextureBuffer.h"
#include "VulkanFramebuffer.h"
#include "VulkanDescriptorPool.h"
#include "VulkanVertexBuffer.h"
#include "VulkanBuffer.h"

#include "Core/Reflection.hpp"

#include <cstring>
#include <utility>

namespace sh::render
{
	VulkanDrawable::VulkanDrawable(VulkanRenderer& renderer) :
		renderer(renderer), 
		mat(nullptr), mesh(nullptr), camera(nullptr),
		localDescSet(),
		bInit(false), bDirty(false), bBufferDirty(false), bPipelineDirty(false)
	{
	}
	VulkanDrawable::VulkanDrawable(VulkanDrawable&& other) noexcept :
		renderer(other.renderer),
		mat(other.mat), mesh(other.mesh), camera(other.camera),
		pipeline(std::move(other.pipeline)),
		localVertBuffer(std::move(other.localVertBuffer)),
		localFragBuffer(std::move(other.localFragBuffer)),
		localDescSet(std::move(other.localDescSet)),
		bInit(other.bInit), bDirty(other.bDirty), bBufferDirty(other.bBufferDirty), bPipelineDirty(other.bPipelineDirty)
	{
		other.mat = nullptr;
		other.mesh = nullptr;
		other.camera = nullptr;

		other.bInit = false;
		other.bDirty = false;
		other.bBufferDirty = false;
		other.bPipelineDirty = false;
	}

	VulkanDrawable::~VulkanDrawable() noexcept
	{
		SH_INFO("~VulkanDrawable()");
		Clean(core::ThreadType::Game);
		Clean(core::ThreadType::Render);
	}

	void VulkanDrawable::Clean(core::ThreadType thr)
	{
		localVertBuffer[thr].clear();
		localFragBuffer[thr].clear();
		localDescSet[thr].reset();
		pipeline[thr].reset();
	}

	void VulkanDrawable::CreateBuffer(core::ThreadType thr)
	{
		VulkanShader* shader = static_cast<VulkanShader*>(mat->GetShader());

		// 버텍스 유니폼
		for (auto& uniformBlock : shader->GetVertexUniforms())
		{
			if (uniformBlock.type == Shader::UniformType::Material)
				continue;

			std::size_t lastOffset = uniformBlock.data.back().offset + uniformBlock.data.back().size;
			std::size_t size = core::Util::AlignTo(lastOffset, uniformBlock.align);

			for (std::size_t thr = 0; thr < localVertBuffer.size(); ++thr)
			{
				auto ptr = static_cast<impl::VulkanBuffer*>(BufferFactory::Create(renderer, size).release());
				localVertBuffer[thr].insert({ uniformBlock.binding, std::unique_ptr<impl::VulkanBuffer>(ptr) });
			}
		}
		// 픽셀 유니폼
		for (auto& uniformBlock : shader->GetFragmentUniforms())
		{
			if (uniformBlock.type == Shader::UniformType::Material)
				continue;

			std::size_t lastOffset = uniformBlock.data.back().offset + uniformBlock.data.back().size;
			std::size_t size = core::Util::AlignTo(lastOffset, uniformBlock.align);

			for (std::size_t thr = 0; thr < localFragBuffer.size(); ++thr)
			{
				auto ptr = static_cast<impl::VulkanBuffer*>(BufferFactory::Create(renderer, size).release());
				localFragBuffer[thr].insert({ uniformBlock.binding, std::unique_ptr<impl::VulkanBuffer>(ptr) });
			}
		}

		auto ptr = static_cast<impl::VulkanUniformBuffer*>(BufferFactory::CreateUniformBuffer(renderer, *this->mat->GetShader(), Shader::UniformType::Object).release());
		localDescSet[thr] = std::unique_ptr<impl::VulkanUniformBuffer>(ptr);
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

		//토폴리지
		impl::VulkanPipeline::Topology topology = impl::VulkanPipeline::Topology::Triangle;
		switch (mesh.GetTopology())
		{
		case Mesh::Topology::Point:
			topology = impl::VulkanPipeline::Topology::Point;
			break;
		case Mesh::Topology::Line:
			topology = impl::VulkanPipeline::Topology::Line;
			break;
		}

		Clean(core::ThreadType::Game);

		int thrCount = bInit ? 1 : 2; // 첫 초기화 시에는 두 번 반복
		for (int thrIdx = 0; thrIdx < thrCount; ++thrIdx)
		{
			core::ThreadType thr = static_cast<core::ThreadType>(thrIdx);
			pipeline[thr] = std::make_unique<impl::VulkanPipeline>(renderer.GetDevice(), vkFrameBuffer->GetRenderPass());
			pipeline[thr]->SetTopology(topology);
			CreateBuffer(thr);

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

				pipeline[thr]->AddBindingDescription(bindings[i]);
				pipeline[thr]->AddAttributeDescription(attrDesc);
			}

			auto result = pipeline[thr]->
				SetShader(static_cast<VulkanShader*>(shader)).
				AddShaderStage(impl::VulkanPipeline::ShaderStage::Vertex).
				AddShaderStage(impl::VulkanPipeline::ShaderStage::Fragment).
				Build(shader->GetPipelineLayout());
			assert(result == VkResult::VK_SUCCESS);
		}

		if (bInit)
		{
			bBufferDirty = true;
			bPipelineDirty = true;
			SetDirty();
		}
		else
			bInit = true;
	}

	void VulkanDrawable::SetUniformData(uint32_t binding, const void* data, Stage stage)
	{
		if (stage == Stage::Vertex)
		{
			auto it = localVertBuffer[core::ThreadType::Game].find(binding);
			if (it == localVertBuffer[core::ThreadType::Game].end())
				return;

			it->second->SetData(data);
			localDescSet[core::ThreadType::Game]->Update(binding, *it->second);

			bBufferDirty = true;
		}
		else if (stage == Stage::Fragment)
		{
			auto it = localFragBuffer[core::ThreadType::Game].find(binding);
			if (it == localFragBuffer[core::ThreadType::Game].end())
				return;

			it->second->SetData(data);
			localDescSet[core::ThreadType::Game]->Update(binding, *it->second);

			bBufferDirty = true;
		}
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
	auto VulkanDrawable::GetPipeline(core::ThreadType thr) const -> impl::VulkanPipeline*
	{
		return pipeline[static_cast<int>(thr)].get();
	}
	auto VulkanDrawable::GetLocalUniformBuffer(core::ThreadType thr) const -> impl::VulkanUniformBuffer*
	{
		return localDescSet[thr].get();
	}
	auto VulkanDrawable::GetDescriptorSet(core::ThreadType thr) const -> VkDescriptorSet
	{
		return localDescSet[thr]->GetVkDescriptorSet();
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
		if (bPipelineDirty)
			std::swap(pipeline[core::ThreadType::Game], pipeline[core::ThreadType::Render]);
		if (bBufferDirty)
		{
			std::swap(localVertBuffer[core::ThreadType::Game], localVertBuffer[core::ThreadType::Render]);
			std::swap(localFragBuffer[core::ThreadType::Game], localFragBuffer[core::ThreadType::Render]);
			std::swap(localDescSet[core::ThreadType::Game], localDescSet[core::ThreadType::Render]);
		}

		bPipelineDirty = false;
		bBufferDirty = false;
		bDirty = false;
	}
}