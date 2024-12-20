﻿#include "pch.h"
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
	SH_RENDER_API VulkanDrawable::VulkanDrawable(VulkanRenderer& renderer) :
		renderer(renderer), 
		mat(nullptr), mesh(nullptr), camera(nullptr),
		localDescSet(),
		bInit(false), bDirty(false), bBufferDirty(false), bPipelineDirty(false)
	{
	}
	SH_RENDER_API VulkanDrawable::VulkanDrawable(VulkanDrawable&& other) noexcept :
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

	SH_RENDER_API VulkanDrawable::~VulkanDrawable() noexcept
	{
		Clean(core::ThreadType::Game);
		Clean(core::ThreadType::Render);
		SH_INFO("~Drawable");
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

			auto ptr = static_cast<impl::VulkanBuffer*>(BufferFactory::Create(renderer, size).release());
			localVertBuffer[thr].insert_or_assign(uniformBlock.binding, std::unique_ptr<impl::VulkanBuffer>(ptr));
		}
		// 픽셀 유니폼
		for (auto& uniformBlock : shader->GetFragmentUniforms())
		{
			if (uniformBlock.type == Shader::UniformType::Material)
				continue;

			std::size_t lastOffset = uniformBlock.data.back().offset + uniformBlock.data.back().size;
			std::size_t size = core::Util::AlignTo(lastOffset, uniformBlock.align);

			auto ptr = static_cast<impl::VulkanBuffer*>(BufferFactory::Create(renderer, size).release());
			localFragBuffer[thr].insert_or_assign(uniformBlock.binding, std::unique_ptr<impl::VulkanBuffer>(ptr));
		}

		auto ptr = static_cast<impl::VulkanUniformBuffer*>(BufferFactory::CreateUniformBuffer(renderer, *this->mat->GetShader(), Shader::UniformType::Object).release());
		localDescSet[thr] = std::unique_ptr<impl::VulkanUniformBuffer>(ptr);
	}
	void VulkanDrawable::BuildPipeline(core::ThreadType thr, impl::VulkanPipeline::Topology topology)
	{
		const impl::VulkanFramebuffer* vkFrameBuffer = nullptr;
		if (camera->GetRenderTexture() == nullptr)
			vkFrameBuffer = static_cast<const impl::VulkanFramebuffer*>(renderer.GetMainFramebuffer());
		else
			vkFrameBuffer = static_cast<const impl::VulkanFramebuffer*>(camera->GetRenderTexture()->GetFramebuffer(core::ThreadType::Game));

		VulkanShader* shader = static_cast<VulkanShader*>(mat->GetShader());
		pipeline[thr] = std::make_unique<impl::VulkanPipeline>(renderer.GetDevice(), vkFrameBuffer->GetRenderPass());
		pipeline[thr]->SetTopology(topology);
		pipeline[thr]->SetShader(shader);
		pipeline[thr]->
			AddShaderStage(impl::VulkanPipeline::ShaderStage::Vertex).
			AddShaderStage(impl::VulkanPipeline::ShaderStage::Fragment);

		//Attribute
		auto& bindings = static_cast<VulkanVertexBuffer*>(mesh->GetVertexBuffer())->bindingDescriptions;
		auto& attrs = static_cast<VulkanVertexBuffer*>(mesh->GetVertexBuffer())->attribDescriptions;

		// binding 0과 attribute 0은 버텍스
		for (int i = 0; i < attrs.size(); ++i)
		{
			auto data = shader->GetAttribute(mesh->attributes[i]->name);
			if (!data)
				continue;
			if (data->typeName != mesh->attributes[i]->typeName)
				continue;

			VkVertexInputAttributeDescription attrDesc = attrs[i];
			attrDesc.location = data->idx;

			pipeline[thr]->AddBindingDescription(bindings[i]);
			pipeline[thr]->AddAttributeDescription(attrDesc);
		}

		pipeline[thr]->SetLineWidth(mesh->lineWidth);

		auto result = pipeline[thr]->Build(shader->GetPipelineLayout());
		assert(result == VkResult::VK_SUCCESS);
	}

	SH_RENDER_API void VulkanDrawable::Build(Camera& camera, const Mesh* mesh, const Material* mat)
	{
		this->mat = mat;
		this->mesh = mesh;
		this->camera = &camera;

		assert(mesh);
		assert(mat);

		VulkanShader* shader = static_cast<VulkanShader*>(mat->GetShader());
		assert(shader);

		// 토폴리지
		switch (mesh->GetTopology())
		{
		case Mesh::Topology::Point:
			topology = impl::VulkanPipeline::Topology::Point;
			break;
		case Mesh::Topology::Line:
			topology = impl::VulkanPipeline::Topology::Line;
			break;
		case Mesh::Topology::Face:
			topology = impl::VulkanPipeline::Topology::Triangle;
			break;
		}

		Clean(core::ThreadType::Game);

		int thrCount = bInit ? 1 : 2; // 첫 초기화 시에는 두 번 반복, 아니면 Game스레드만
		for (int thrIdx = 0; thrIdx < thrCount; ++thrIdx)
		{
			core::ThreadType thr = static_cast<core::ThreadType>(thrIdx);
			CreateBuffer(thr);
			BuildPipeline(thr, topology);
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

	SH_RENDER_API void VulkanDrawable::SetUniformData(uint32_t binding, const void* data, Stage stage)
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

	SH_RENDER_API auto VulkanDrawable::GetMaterial() const -> const Material*
	{
		return mat;
	}
	SH_RENDER_API auto VulkanDrawable::GetMesh() const -> const Mesh*
	{
		return mesh;
	}
	SH_RENDER_API auto VulkanDrawable::GetCamera() const -> Camera*
	{
		return camera;
	}
	SH_RENDER_API auto VulkanDrawable::GetPipeline(core::ThreadType thr) const -> impl::VulkanPipeline*
	{
		return pipeline[static_cast<int>(thr)].get();
	}
	SH_RENDER_API auto VulkanDrawable::GetLocalUniformBuffer(core::ThreadType thr) const -> impl::VulkanUniformBuffer*
	{
		return localDescSet[thr].get();
	}
	SH_RENDER_API auto VulkanDrawable::GetDescriptorSet(core::ThreadType thr) const -> VkDescriptorSet
	{
		return localDescSet[thr]->GetVkDescriptorSet();
	}

	SH_RENDER_API void VulkanDrawable::SetDirty()
	{
		if (bDirty)
			return;

		renderer.GetThreadSyncManager().PushSyncable(*this);
		bDirty = true;
	}

	SH_RENDER_API void VulkanDrawable::Sync()
	{
		if (bBufferDirty)
		{
			std::swap(localVertBuffer[core::ThreadType::Game], localVertBuffer[core::ThreadType::Render]);
			std::swap(localFragBuffer[core::ThreadType::Game], localFragBuffer[core::ThreadType::Render]);
			std::swap(localDescSet[core::ThreadType::Game], localDescSet[core::ThreadType::Render]);
		}
		if (bPipelineDirty)
		{
			// 새로 빌드 했다는 뜻이므로 버퍼 교환 후 파이프라인을 새로 빌드 해준다.
			std::swap(pipeline[core::ThreadType::Game], pipeline[core::ThreadType::Render]);

			BuildPipeline(core::ThreadType::Game, topology);
			CreateBuffer(core::ThreadType::Game);
		}

		bPipelineDirty = false;
		bBufferDirty = false;
		bDirty = false;
	}
}