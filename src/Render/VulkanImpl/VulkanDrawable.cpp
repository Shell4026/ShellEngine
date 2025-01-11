#include "VulkanDrawable.h"

#include "Material.h"
#include "Mesh.h"
#include "BufferFactory.h"

#include "VulkanContext.h"
#include "VulkanShaderPass.h"
#include "VulkanFramebuffer.h"
#include "VulkanVertexBuffer.h"
#include "VulkanBuffer.h"
#include "VulkanPipelineManager.h"

#include "Core/Reflection.hpp"
#include "Core/ThreadSyncManager.h"

namespace sh::render::vk
{
	SH_RENDER_API VulkanDrawable::VulkanDrawable(const VulkanContext& context) :
		context(context),
		mat(nullptr), mesh(nullptr), camera(nullptr),
		bInit(false), bDirty(false), bBufferDirty(false)
	{
	}
	SH_RENDER_API VulkanDrawable::VulkanDrawable(VulkanDrawable&& other) noexcept :
		context(other.context),
		mat(other.mat), mesh(other.mesh), camera(other.camera),
		perPassData(std::move(other.perPassData)),
		bInit(other.bInit), bDirty(other.bDirty), bBufferDirty(other.bBufferDirty)
	{
		other.mat = nullptr;
		other.mesh = nullptr;
		other.camera = nullptr;

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
		perPassData[thr].clear();
	}

	void VulkanDrawable::CreateBuffers(core::ThreadType thr)
	{
		Shader* shader = mat->GetShader();

		std::size_t passIdx = 0;
		for (auto& shaderPass : shader->GetPasses())
		{
			auto& uniformData = perPassData[thr][passIdx];
			// 버텍스 유니폼
			for (auto& uniformBlock : shaderPass->GetVertexUniforms())
			{
				if (uniformBlock.type == ShaderPass::UniformType::Material)
					continue;

				std::size_t lastOffset = uniformBlock.data.back().offset + uniformBlock.data.back().size;
				std::size_t size = core::Util::AlignTo(lastOffset, uniformBlock.align);

				auto ptr = static_cast<VulkanBuffer*>(BufferFactory::Create(context, size).release());
				uniformData.vertShaderData.insert_or_assign(uniformBlock.binding, std::unique_ptr<VulkanBuffer>(ptr));
			}
			// 픽셀 유니폼
			for (auto& uniformBlock : shaderPass->GetFragmentUniforms())
			{
				if (uniformBlock.type == ShaderPass::UniformType::Material)
					continue;

				std::size_t lastOffset = uniformBlock.data.back().offset + uniformBlock.data.back().size;
				std::size_t size = core::Util::AlignTo(lastOffset, uniformBlock.align);

				auto ptr = static_cast<VulkanBuffer*>(BufferFactory::Create(context, size).release());
				uniformData.fragShaderData.insert_or_assign(uniformBlock.binding, std::unique_ptr<VulkanBuffer>(ptr));
			}

			auto ptr = static_cast<VulkanUniformBuffer*>(BufferFactory::CreateUniformBuffer(context, *shaderPass, ShaderPass::UniformType::Object).release());
			uniformData.uniformBuffer = std::unique_ptr<VulkanUniformBuffer>(ptr);
		}
	}
	void VulkanDrawable::GetPipelineFromManager(core::ThreadType thr)
	{
		const VulkanFramebuffer* vkFrameBuffer = nullptr;
		if (camera->GetRenderTexture() == nullptr)
			vkFrameBuffer = static_cast<const VulkanFramebuffer*>(context.GetMainFramebuffer());
		else
			vkFrameBuffer = static_cast<const VulkanFramebuffer*>(camera->GetRenderTexture()->GetFramebuffer(core::ThreadType::Game));

		std::size_t passIdx = 0;
		for (auto& shaderPass : mat->GetShader()->GetPasses())
			perPassData[thr][passIdx++].pipelineHandle = context.GetPipelineManager().GetPipelineHandle(vkFrameBuffer->GetRenderPass(), static_cast<VulkanShaderPass&>(*shaderPass), *mesh);
	}

	SH_RENDER_API void VulkanDrawable::Build(Camera& camera, Mesh* mesh, Material* mat)
	{
		this->mat = mat;
		this->mesh = mesh;
		this->camera = &camera;
		assert(mesh);
		assert(mat);

		Shader* shader = mat->GetShader();
		assert(shader);
		if (!core::IsValid(shader))
			return;

		Clean(core::ThreadType::Game);
		perPassData[core::ThreadType::Game].resize(shader->GetPasses().size());
		GetPipelineFromManager(core::ThreadType::Game);
		CreateBuffers(core::ThreadType::Game);
		if (!bInit)
		{
			Clean(core::ThreadType::Render);
			perPassData[core::ThreadType::Render].resize(shader->GetPasses().size());
			GetPipelineFromManager(core::ThreadType::Render);
			CreateBuffers(core::ThreadType::Render);
		}
		if (bInit)
		{
			bRecreateBufferDirty = true;
			SetDirty();
		}
		else
			bInit = true;
	}

	SH_RENDER_API void VulkanDrawable::SetUniformData(std::size_t passIdx, uint32_t binding, const void* data, Stage stage)
	{
		auto& passData = perPassData[core::ThreadType::Game][passIdx];
		if (stage == Stage::Vertex)
		{
			auto it = passData.vertShaderData.find(binding);
			if (it == passData.vertShaderData.end())
				return;

			it->second->SetData(data);
			passData.uniformBuffer->Update(binding, *it->second);
		}
		else if (stage == Stage::Fragment)
		{
			auto it = passData.fragShaderData.find(binding);
			if (it == passData.fragShaderData.end())
				return;

			it->second->SetData(data);
			passData.uniformBuffer->Update(binding, *it->second);
		}
		bBufferDirty = true;
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
	SH_RENDER_API auto VulkanDrawable::GetPipelineHandle(std::size_t passIdx, core::ThreadType thr) const -> uint64_t
	{
		return perPassData[thr][passIdx].pipelineHandle;
	}
	SH_RENDER_API auto VulkanDrawable::GetLocalUniformBuffer(std::size_t passIdx, core::ThreadType thr) const -> VulkanUniformBuffer*
	{
		return static_cast<VulkanUniformBuffer*>(perPassData[thr][passIdx].uniformBuffer.get());
	}
	SH_RENDER_API auto VulkanDrawable::GetDescriptorSet(std::size_t passIdx, core::ThreadType thr) const -> VkDescriptorSet
	{
		return GetLocalUniformBuffer(passIdx, thr)->GetVkDescriptorSet();
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
		if (bBufferDirty || bRecreateBufferDirty)
		{
			std::swap(perPassData[core::ThreadType::Game], perPassData[core::ThreadType::Render]);
			if (bRecreateBufferDirty)
			{
				Clean(core::ThreadType::Game);
				Build(*camera, mesh, mat);
			}
		}
		bRecreateBufferDirty = false;
		bBufferDirty = false;
		bDirty = false;
	}

	SH_RENDER_API bool VulkanDrawable::CheckAssetValid() const
	{
		return core::IsValid(mesh) && core::IsValid(mat) && core::IsValid(mat->GetShader());
	}
}