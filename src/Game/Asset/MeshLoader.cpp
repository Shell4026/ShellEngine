#include "Asset/MeshLoader.h"

#include "Core/Logger.h"

#include "Render/SkinnedMesh.h"

#include <cstring>
namespace sh::game
{
	MeshLoader::MeshLoader(const render::IRenderContext& ctx) :
		ctx(ctx)
	{
	}
	SH_GAME_API auto MeshLoader::Load(const std::filesystem::path& filePath) const -> core::SObject*
	{
		return nullptr;
	}
	SH_GAME_API auto MeshLoader::Load(const core::Asset& asset) const -> core::SObject*
	{
		if (std::strcmp(asset.GetType(), GetAssetName()) != 0)
			return nullptr;

		const MeshAsset& meshAsset = static_cast<const MeshAsset&>(asset);
		const MeshAsset::Header header = meshAsset.GetHeader();
		const MeshAsset::MeshData data = meshAsset.GetData();

		const size_t vertexBytes = header.vertexCount * sizeof(render::Mesh::Vertex);
		const size_t indexBytes = header.indexCount * sizeof(uint32_t);
		const size_t boneVertexBytes = header.boneVertexCount * sizeof(render::SkinnedMesh::BoneVertex);
		const size_t subMeshBytes = header.subMeshCount * sizeof(render::SubMesh);
		const size_t ibmBytes = header.ibmCount * sizeof(glm::mat4);

		if (data.size < vertexBytes + indexBytes + boneVertexBytes + subMeshBytes + ibmBytes)
		{
			SH_ERROR_FORMAT("Invalid mesh asset!: {}", asset.GetAssetUUID().ToString());
			return nullptr;
		}

		std::vector<render::Mesh::Vertex> vertices(header.vertexCount);
		std::vector<uint32_t> indices(header.indexCount);
		std::vector<render::SubMesh> subMeshes(header.subMeshCount);

		const uint8_t* cursor = data.dataPtr;
		std::memcpy(vertices.data(), cursor, vertexBytes);
		cursor += vertexBytes;
		std::memcpy(indices.data(), cursor, indexBytes);
		cursor += indexBytes;

		render::Mesh* const mesh = header.boneVertexCount > 0 ?
			core::SObject::Create<render::SkinnedMesh>() : core::SObject::Create<render::Mesh>();

		mesh->SetUUID(asset.GetAssetUUID());
		mesh->SetVertex(std::move(vertices));
		mesh->SetIndices(std::move(indices));

		if (header.boneVertexCount > 0)
		{
			render::SkinnedMesh* const skinnedMesh = static_cast<render::SkinnedMesh*>(mesh);

			std::vector<render::SkinnedMesh::BoneVertex> boneVerts(header.boneVertexCount);
			std::memcpy(boneVerts.data(), cursor, boneVertexBytes);
			skinnedMesh->SetBoneVertices(std::move(boneVerts));
			cursor += boneVertexBytes;

			std::memcpy(subMeshes.data(), cursor, subMeshBytes);
			cursor += subMeshBytes;

			if (header.ibmCount > 0)
			{
				std::vector<glm::mat4> ibms(header.ibmCount);
				std::memcpy(ibms.data(), cursor, ibmBytes);
				skinnedMesh->SetInverseBindMatrices(std::move(ibms));
			}
		}
		else
		{
			std::memcpy(subMeshes.data(), cursor, subMeshBytes);
		}

		mesh->SetSubMeshes(std::move(subMeshes));
		mesh->Build(ctx);

		return mesh;
	}
	SH_GAME_API auto MeshLoader::GetAssetName() const -> const char*
	{
		return MeshAsset::ASSET_NAME;
	}
}//namespace
