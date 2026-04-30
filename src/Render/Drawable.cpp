#include "Drawable.h"
#include "SkinnedMesh.h"

#include "Core/ThreadSyncManager.h"

#include <cassert>
#include <variant>
namespace sh::render
{
	Drawable::Drawable(const Material& mat, const Mesh& mesh) :
		mat(&mat), mesh(&mesh)
	{
		topology[core::ThreadType::Game] = Mesh::Topology::Face;
		topology[core::ThreadType::Render] = Mesh::Topology::Face;

		modelMatrix[core::ThreadType::Game] = glm::mat4{ 1.f };
		modelMatrix[core::ThreadType::Render] = glm::mat4{ 1.f };

		priority[core::ThreadType::Game] = 0;
		priority[core::ThreadType::Render] = 0;

		if (mesh.GetType().IsChildOf(SkinnedMesh::GetStaticType()))
			bSkinned = true;
	}
	Drawable::Drawable(Drawable&& other) noexcept :
		mat(other.mat), mesh(other.mesh), modelMatrix(other.modelMatrix),
		materialData(std::move(other.materialData)),
		renderTag(other.renderTag),
		priority(other.priority),
		topology(other.topology),
		subMeshIndex(other.subMeshIndex),
		syncDatas(other.syncDatas),
		bSkinned(other.bSkinned),
		bDirty(other.bDirty),
		bMatrixDirty(other.bMatrixDirty)
	{
		other.bDirty = false;
	}
	Drawable::~Drawable()
	{
	}

	SH_RENDER_API void Drawable::Build(const IRenderContext& context)
	{
		this->context = &context;
		if (core::IsValid(mat) && core::IsValid(mat->GetShader()))
			materialData.Create(context, *mat->GetShader(), true); // sync타이밍에 이뤄짐
	}

	SH_RENDER_API void Drawable::SetMesh(const Mesh& mesh)
	{
		if (this->mesh == &mesh)
			return;
		SyncData data{};
		data.changed = &mesh;
		syncDatas[1] = data;

		SyncDirty();
	}
	SH_RENDER_API void Drawable::SetMaterial(const Material& mat)
	{
		if (this->mat == &mat)
			return;
		SyncData data{};
		data.changed = &mat;
		syncDatas[0]= data;

		SyncDirty();
		if (context != nullptr && core::IsValid(mat.GetShader()))
			materialData.Create(*context, *mat.GetShader(), true); // 얘도 sync타이밍에 이뤄짐
	}
	SH_RENDER_API void Drawable::SetModelMatrix(const glm::mat4& mat)
	{
		modelMatrix[core::ThreadType::Game] = mat;
		bMatrixDirty = true;
		SyncDirty();
	}
	SH_RENDER_API void Drawable::SetRenderTagId(uint32_t tagId)
	{
		renderTag = tagId;
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
	SH_RENDER_API void Drawable::SetPriority(int priority)
	{
		assert(core::ThreadSyncManager::IsMainThread());
		if (this->priority[core::ThreadType::Game] != priority)
		{
			this->priority[core::ThreadType::Game] = priority;
			syncDatas[3].changed = priority;
			SyncDirty();
		}
	}
	SH_RENDER_API void Drawable::SetSubMeshIndex(uint32_t idx)
	{
		subMeshIndex = idx;
	}

	SH_RENDER_API auto Drawable::CheckAssetValid() const -> bool
	{
		return core::IsValid(mesh) && core::IsValid(mat) && core::IsValid(mesh);
	}

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
			if (std::holds_alternative<std::monostate>(syncData.changed))
			{
				continue;
			}
			else if (std::holds_alternative<const Material*>(syncData.changed)) // 메테리얼이 변경됨
			{
				mat = std::get<const Material*>(syncData.changed);
			}
			else if (std::holds_alternative<const Mesh*>(syncData.changed))
			{
				mesh = std::get<const Mesh*>(syncData.changed);
				if (mesh->GetType().IsChildOf(SkinnedMesh::GetStaticType()))
					bSkinned = true;
			}
			else if (std::holds_alternative<Mesh::Topology>(syncData.changed))
			{
				topology[core::ThreadType::Render] = std::get<Mesh::Topology>(syncData.changed);
			}
			else if (std::holds_alternative<int>(syncData.changed))
			{
				priority[core::ThreadType::Render] = std::get<int>(syncData.changed);
			}
			syncData.changed = std::monostate{};
		}
		if (bMatrixDirty)
			std::swap(modelMatrix[core::ThreadType::Game], modelMatrix[core::ThreadType::Render]);
		bMatrixDirty = false;
		bDirty = false;
	}
}//namespace