#pragma once
#include "Export.h"
#include "Mesh.h"

#include <glm/glm.hpp>
#include <vector>

namespace sh::render
{
	/// @brief 스켈레탈 애니메이션용 메쉬.
	/// @brief Mesh::Vertex 데이터에 더해 bone indices/weights를 별도 벡터로 보관한다.
	/// @brief VulkanSkinnedVertexBuffer가 두 번째 binding으로 업로드한다.
	class SkinnedMesh : public Mesh
	{
		SCLASS(SkinnedMesh)
	public:
		struct BoneVertex
		{
			glm::ivec4 boneIndices{ 0 };
			glm::vec4 boneWeights{ 0.0f };
		};

		static constexpr uint8_t BONE_INDEX_ID = 4;
		static constexpr uint8_t BONE_WEIGHT_ID = 5;
	public:
		SH_RENDER_API SkinnedMesh();
		SH_RENDER_API SkinnedMesh(const SkinnedMesh& other);
		SH_RENDER_API SkinnedMesh(SkinnedMesh&& other) noexcept;
		SH_RENDER_API ~SkinnedMesh();

		/// @brief GPU 버퍼 생성. VulkanSkinnedVertexBuffer를 생성한다.
		SH_RENDER_API void Build(const IRenderContext& context) override;

		SH_RENDER_API void SetBoneVertices(std::vector<BoneVertex> bv) { boneVerts = std::move(bv); }
		SH_RENDER_API void SetInverseBindMatrices(std::vector<glm::mat4> ibms) { inverseBindMatrices = std::move(ibms); }

		SH_RENDER_API auto GetBoneVertices() const -> const std::vector<BoneVertex>& { return boneVerts; }
		SH_RENDER_API auto GetInverseBindMatrices() const -> const std::vector<glm::mat4>& { return inverseBindMatrices; }
	private:
		std::vector<BoneVertex> boneVerts;
		std::vector<glm::mat4> inverseBindMatrices;
	};
}//namespace
