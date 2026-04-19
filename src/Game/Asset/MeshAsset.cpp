#include "Asset/MeshAsset.h"

namespace sh::game
{
	MeshAsset::MeshAsset() :
		Asset(ASSET_NAME)
	{
	}
	MeshAsset::MeshAsset(const render::Mesh& mesh) :
		Asset(ASSET_NAME)
	{
		meshPtr = &mesh;
		assetUUID = meshPtr->GetUUID();
	}
	MeshAsset::MeshAsset(const render::SkinnedMesh& mesh) :
		Asset(ASSET_NAME)
	{
		meshPtr = &mesh;
		assetUUID = meshPtr->GetUUID();
	}
	SH_GAME_API void MeshAsset::SetAsset(const core::SObject& obj)
	{
		if (obj.GetType() != render::Mesh::GetStaticType() &&
			obj.GetType() != render::SkinnedMesh::GetStaticType())
			return;

		meshPtr = static_cast<const render::Mesh*>(&obj);
		assetUUID = meshPtr->GetUUID();
	}
	SH_GAME_API auto MeshAsset::GetHeader() const -> const Header&
	{
		return header;
	}
	SH_GAME_API auto MeshAsset::GetData() const -> MeshData
	{
		MeshData meshData{};
		meshData.dataPtr = data.data() + sizeof(Header);
		meshData.size = data.size() - sizeof(Header);
		return meshData;
	}
	SH_GAME_API void MeshAsset::SetAssetData() const
	{
		if (!meshPtr.IsValid())
			return;

		const bool bSkinned = (meshPtr->GetType() == render::SkinnedMesh::GetStaticType());
		const render::SkinnedMesh* skinnedPtr = bSkinned ? static_cast<const render::SkinnedMesh*>(meshPtr.Get()) : nullptr;

		Header header{};
		header.vertexCount = meshPtr->GetVertexCount();
		header.indexCount = meshPtr->GetIndices().size();
		header.boneVertexCount = bSkinned ? skinnedPtr->GetBoneVertices().size() : 0;
		header.subMeshCount = meshPtr->GetSubMeshes().size();

		const size_t vertexBytes = header.vertexCount * sizeof(render::Mesh::Vertex);
		const size_t indexBytes = header.indexCount * sizeof(uint32_t);
		const size_t boneVertexBytes = header.boneVertexCount * sizeof(render::SkinnedMesh::BoneVertex);
		const size_t subMeshBytes = header.subMeshCount * sizeof(render::SubMesh);

		data.resize(sizeof(Header) + vertexBytes + indexBytes + boneVertexBytes + subMeshBytes);

		uint8_t* cursor = data.data();
		std::memcpy(cursor, &header, sizeof(Header));
		cursor += sizeof(Header);
		std::memcpy(cursor, meshPtr->GetVertex().data(), vertexBytes);
		cursor += vertexBytes;
		std::memcpy(cursor, meshPtr->GetIndices().data(), indexBytes);
		cursor += indexBytes;
		if (bSkinned)
		{
			std::memcpy(cursor, skinnedPtr->GetBoneVertices().data(), boneVertexBytes);
			cursor += boneVertexBytes;
		}
		std::memcpy(cursor, meshPtr->GetSubMeshes().data(), subMeshBytes);
	}
	SH_GAME_API auto MeshAsset::ParseAssetData() -> bool
	{
		meshPtr.Reset();

		if (data.size() < sizeof(Header))
			return false;

		std::memcpy(&header, data.data(), sizeof(Header));
		return true;
	}
}//namespace
