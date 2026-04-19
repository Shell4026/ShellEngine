#pragma once
#include "Export.h"
#include "Mesh.h"
#include "Material.h"
#include "MaterialData.h"

#include "Core/SObject.h"
#include "Core/NonCopyable.h"
#include "Core/ISyncable.h"

#include <vector>
#include <variant>
namespace sh::render
{
	class Drawable : 
		public core::SObject, 
		public core::INonCopyable,
		public core::ISyncable,
		public IRenderResource
	{
		SCLASS(Drawable)
	public:
		SH_RENDER_API Drawable(const Material& mat, const Mesh& mesh);
		SH_RENDER_API Drawable(Drawable&& other) noexcept;
		SH_RENDER_API virtual ~Drawable();

		SH_RENDER_API void Build(const IRenderContext& context) override;

		/// @brief 메쉬를 지정한다.
		/// @param mesh 메쉬
		SH_RENDER_API void SetMesh(const Mesh& mesh);
		SH_RENDER_API void SetMaterial(const Material& mat);
		SH_RENDER_API void SetModelMatrix(const glm::mat4& mat);
		SH_RENDER_API void SetRenderTagId(uint32_t tagId);
		SH_RENDER_API void SetTopology(Mesh::Topology topology);
		SH_RENDER_API void SetPriority(int priority);
		SH_RENDER_API void SetSubMeshIndex(int idx);

		SH_RENDER_API auto CheckAssetValid() const -> bool;

		SH_RENDER_API auto GetMesh() const -> const Mesh* { return mesh; }
		SH_RENDER_API auto GetMaterial() const -> const Material* { return mat; }
		SH_RENDER_API auto GetMaterialData() const -> const MaterialData& { return materialData; }
		SH_RENDER_API auto GetMaterialData() -> MaterialData& { return materialData; }
		SH_RENDER_API auto GetModelMatrix(core::ThreadType thr) const -> const glm::mat4& { return modelMatrix[thr]; }
		SH_RENDER_API auto GetRenderTagId() const -> uint32_t { return renderTag; }
		SH_RENDER_API auto GetTopology(core::ThreadType thr = core::ThreadType::Game) const -> Mesh::Topology { return topology[thr]; }
		SH_RENDER_API auto GetPriority(core::ThreadType thr = core::ThreadType::Game) const -> int { return priority[thr]; }
		SH_RENDER_API auto GetSubMeshIndex() const -> int { return subMeshIndex; }
	protected:
		SH_RENDER_API void SyncDirty() override;
		SH_RENDER_API void Sync() override;
	private:
		const IRenderContext* context = nullptr;

		PROPERTY(mat)
		const Material* mat = nullptr;
		PROPERTY(mesh)
		const Mesh* mesh = nullptr;

		MaterialData materialData;

		core::SyncArray<glm::mat4> modelMatrix;
		uint32_t renderTag = 1;
		core::SyncArray<Mesh::Topology> topology;
		core::SyncArray<int> priority;

		struct SyncData
		{
			std::variant<std::monostate, const Material*, const Mesh*, Mesh::Topology, int> changed;
		};
		std::array<SyncData, 4> syncDatas;

		int subMeshIndex = -1;

		bool bDirty = false;
		bool bMatrixDirty = false;
	};
}//namespace