﻿#include "Drawable.h"

#include "Core/ThreadSyncManager.h"

#include <cassert>

namespace sh::render
{
	SH_RENDER_API void Drawable::SyncDirty()
	{
		if (bDirty)
			return;

		core::ThreadSyncManager::PushSyncable(*this);

		bDirty = true;
	}
	SH_RENDER_API void Drawable::Sync()
	{
		for (auto& syncData : syncDatas)
		{
			std::size_t index = syncData.changed.index();
			if (index == 0)
				continue;
			else if (index == 1)
			{
				this->mat = std::get<1>(syncData.changed);
				if (core::IsValid(mat->GetShader()))
					materialData.Create(*context, *mat->GetShader(), true);
			}
			else if (index == 2)
			{
				this->mesh = std::get<2>(syncData.changed);
			}
			else
			{
				topology[core::ThreadType::Render] = std::get<3>(syncData.changed);
			}
			syncData.changed = std::monostate{};
		}
		if (bMatrixDirty)
			std::swap(modelMatrix[core::ThreadType::Game], modelMatrix[core::ThreadType::Render]);
		if (bLightDirty)
			std::swap(light[core::ThreadType::Game], light[core::ThreadType::Render]);
		bMatrixDirty = false;
		bLightDirty = false;
		bDirty = false;
	}

	Drawable::Drawable(const Material& material, const Mesh& mesh) :
		mat(&material), mesh(&mesh)
	{
		topology[core::ThreadType::Game] = Mesh::Topology::Face;
		topology[core::ThreadType::Render] = Mesh::Topology::Face;

		modelMatrix[core::ThreadType::Game] = glm::mat4{ 1.f };
		modelMatrix[core::ThreadType::Render] = glm::mat4{ 1.f };

		SH_INFO("Creation");
	}
	Drawable::Drawable(Drawable&& other) noexcept :
		mat(other.mat), mesh(other.mesh), modelMatrix(other.modelMatrix),
		materialData(std::move(other.materialData)), 
		light(other.light),
		renderTag(other.renderTag)
	{
		other.bDirty = false;
		topology[core::ThreadType::Game] = other.topology[core::ThreadType::Game];
		topology[core::ThreadType::Render] = other.topology[core::ThreadType::Render];
		SH_INFO("Creation");
	}
	Drawable::~Drawable()
	{
		SH_INFO_FORMAT("~Drawable: {}", (void*)this);
	}

	SH_RENDER_API void Drawable::Build(const IRenderContext& context)
	{
		this->context = &context;

		assert(core::IsValid(mat->GetShader()));
		if (!core::IsValid(mat->GetShader()))
			return;

		materialData.Create(context, *mat->GetShader(), true);
	}

	SH_RENDER_API void Drawable::SetMesh(const Mesh& mesh)
	{
		SyncData data{};
		data.changed = &mesh;
		syncDatas[1] = data;

		SyncDirty();
	}
	SH_RENDER_API void Drawable::SetMaterial(const Material& mat)
	{
		SyncData data{};
		data.changed = &mat;
		syncDatas[0]= data;

		SyncDirty();
	}

	SH_RENDER_API auto Drawable::GetMaterial() const -> const Material*
	{
		return mat;
	}
	SH_RENDER_API auto Drawable::GetMesh() const -> const Mesh*
	{
		return mesh;
	}
	SH_RENDER_API auto Drawable::GetMaterialData() const -> const MaterialData&
	{
		return materialData;
	}
	SH_RENDER_API auto Drawable::GetMaterialData() -> MaterialData&
	{
		return materialData;
	}

	SH_RENDER_API void Drawable::SetModelMatrix(const glm::mat4& mat)
	{
		modelMatrix[core::ThreadType::Game] = mat;
		bMatrixDirty = true;
		SyncDirty();
	}
	SH_RENDER_API auto Drawable::GetModelMatrix(core::ThreadType thr) const -> const glm::mat4&
	{
		return modelMatrix[thr];
	}

	SH_RENDER_API void Drawable::SetLightData(const Light& lightData)
	{
		light[core::ThreadType::Game] = lightData;
		bLightDirty = true;
		SyncDirty();
	}
	SH_RENDER_API auto Drawable::GetLightData(core::ThreadType thr) const -> const Light&
	{
		return light[thr];
	}

	SH_RENDER_API auto Drawable::CheckAssetValid() const -> bool
	{
		return core::IsValid(mat) && core::IsValid(mesh);
	}
	SH_RENDER_API void Drawable::SetRenderTagId(uint32_t tagId)
	{
		renderTag = tagId;
	}
	SH_RENDER_API auto Drawable::GetRenderTagId() const -> uint32_t
	{
		return renderTag;
	}

	SH_RENDER_API void Drawable::SetTopology(Mesh::Topology topology)
	{
		assert(core::ThreadSyncManager::IsMainThread());
		if (this->topology[core::ThreadType::Game] != topology)
		{
			this->topology[core::ThreadType::Game] = topology;
			syncDatas[2].changed = topology;
			SyncDirty();
		}
	}
	SH_RENDER_API auto Drawable::GetTopology(core::ThreadType thr) const -> Mesh::Topology
	{
		return topology[thr];
	}
}//namespace
