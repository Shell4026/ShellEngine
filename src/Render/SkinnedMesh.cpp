#include "SkinnedMesh.h"
#include "VertexBufferFactory.h"

namespace sh::render
{
	SkinnedMesh::SkinnedMesh() = default;

	SkinnedMesh::SkinnedMesh(const SkinnedMesh& other) :
		Mesh(other),
		boneVerts(other.boneVerts),
		inverseBindMatrices(other.inverseBindMatrices)
	{
	}
	SkinnedMesh::SkinnedMesh(SkinnedMesh&& other) noexcept :
		Mesh(std::move(other)),
		boneVerts(std::move(other.boneVerts)),
		inverseBindMatrices(std::move(other.inverseBindMatrices))
	{
	}
	SkinnedMesh::~SkinnedMesh() = default;

	SH_RENDER_API void SkinnedMesh::Build(const IRenderContext& context)
	{
		CreateFace();

		SetVertexBuffer(VertexBufferFactory::CreateSkinned(context, *this));
	}
}//namespace