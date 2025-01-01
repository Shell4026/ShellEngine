#include "pch.h"
#include "VulkanDrawable.h"

#include "Material.h"
#include "Mesh.h"
#include "BufferFactory.h"

#include "VulkanContext.h"
#include "VulkanShader.h"
#include "VulkanTextureBuffer.h"
#include "VulkanFramebuffer.h"
#include "VulkanDescriptorPool.h"
#include "VulkanVertexBuffer.h"
#include "VulkanBuffer.h"
#include "VulkanPipelineManager.h"

#include "Core/Reflection.hpp"
#include "Core/ThreadSyncManager.h"

#include <cstring>
#include <utility>

namespace sh::render::vk
{
	SH_RENDER_API VulkanDrawable::VulkanDrawable(const VulkanContext& context) :
		context(context),
		mat(nullptr), mesh(nullptr), camera(nullptr),
		localDescSet(),
		bInit(false), bDirty(false), bBufferDirty(false)
	{
	}
	SH_RENDER_API VulkanDrawable::VulkanDrawable(VulkanDrawable&& other) noexcept :
		context(other.context),
		mat(other.mat), mesh(other.mesh), camera(other.camera),
		pipelineHandle(other.pipelineHandle),
		localVertBuffer(std::move(other.localVertBuffer)),
		localFragBuffer(std::move(other.localFragBuffer)),
		localDescSet(std::move(other.localDescSet)),
		bInit(other.bInit), bDirty(other.bDirty), bBufferDirty(other.bBufferDirty)
	{
		other.mat = nullptr;
		other.mesh = nullptr;
		other.camera = nullptr;
		other.pipelineHandle = 0;

		other.bInit = false;
		other.bDirty = false;
		other.bBufferDirty = false;
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
	}

	void VulkanDrawable::CreateBuffers(core::ThreadType thr)
	{
		VulkanShader* shader = static_cast<VulkanShader*>(mat->GetShader());

		// 버텍스 유니폼
		for (auto& uniformBlock : shader->GetVertexUniforms())
		{
			if (uniformBlock.type == Shader::UniformType::Material)
				continue;

			std::size_t lastOffset = uniformBlock.data.back().offset + uniformBlock.data.back().size;
			std::size_t size = core::Util::AlignTo(lastOffset, uniformBlock.align);

			auto ptr = static_cast<VulkanBuffer*>(BufferFactory::Create(context, size).release());
			localVertBuffer[thr].insert_or_assign(uniformBlock.binding, std::unique_ptr<VulkanBuffer>(ptr));
		}
		// 픽셀 유니폼
		for (auto& uniformBlock : shader->GetFragmentUniforms())
		{
			if (uniformBlock.type == Shader::UniformType::Material)
				continue;

			std::size_t lastOffset = uniformBlock.data.back().offset + uniformBlock.data.back().size;
			std::size_t size = core::Util::AlignTo(lastOffset, uniformBlock.align);

			auto ptr = static_cast<VulkanBuffer*>(BufferFactory::Create(context, size).release());
			localFragBuffer[thr].insert_or_assign(uniformBlock.binding, std::unique_ptr<VulkanBuffer>(ptr));
		}

		auto ptr = static_cast<VulkanUniformBuffer*>(BufferFactory::CreateUniformBuffer(context, *this->mat->GetShader(), Shader::UniformType::Object).release());
		localDescSet[thr] = std::unique_ptr<VulkanUniformBuffer>(ptr);
	}
	void VulkanDrawable::GetPipelineFromManager()
	{
		const VulkanFramebuffer* vkFrameBuffer = nullptr;
		if (camera->GetRenderTexture() == nullptr)
			vkFrameBuffer = static_cast<const VulkanFramebuffer*>(context.GetMainFramebuffer());
		else
			vkFrameBuffer = static_cast<const VulkanFramebuffer*>(camera->GetRenderTexture()->GetFramebuffer(core::ThreadType::Game));

		VulkanShader* shader = static_cast<VulkanShader*>(mat->GetShader());
		assert(shader != nullptr);

		pipelineHandle = context.GetPipelineManager().GetPipelineHandle(vkFrameBuffer->GetRenderPass(), *shader, *mesh);
	}

	SH_RENDER_API void VulkanDrawable::Build(Camera& camera, Mesh* mesh, Material* mat)
	{
		this->mat = mat;
		this->mesh = mesh;
		this->camera = &camera;
		assert(mesh);
		assert(mat);

		VulkanShader* shader = static_cast<VulkanShader*>(mat->GetShader());
		assert(shader);

		Clean(core::ThreadType::Game);

		GetPipelineFromManager();

		int thrCount = bInit ? 1 : 2; // 첫 초기화 시에는 두 스레드, 아니면 Game스레드만
		for (int thrIdx = 0; thrIdx < thrCount; ++thrIdx)
		{
			core::ThreadType thr = static_cast<core::ThreadType>(thrIdx);
			CreateBuffers(thr);
		}

		if (bInit)
		{
			bBufferDirty = true;
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
	SH_RENDER_API auto VulkanDrawable::GetPipelineHandle() const -> uint64_t
	{
		return pipelineHandle;
	}
	SH_RENDER_API auto VulkanDrawable::GetLocalUniformBuffer(core::ThreadType thr) const -> VulkanUniformBuffer*
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

		core::ThreadSyncManager::GetInstance()->PushSyncable(*this);
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

		bBufferDirty = false;
		bDirty = false;
	}

	SH_RENDER_API bool VulkanDrawable::CheckAssetValid() const
	{
		return core::IsValid(mesh) && core::IsValid(mat) && core::IsValid(mat->GetShader());
	}
}