#include "MeshAsset.h"

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
	SH_GAME_API void MeshAsset::SetAsset(const core::SObject& obj)
	{
		if (obj.GetType() != render::Mesh::GetStaticType())
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

		Header header{};
		header.vertexCount = meshPtr->GetVertexCount();
		header.indexCount = meshPtr->GetIndices().size();

		size_t totalSize = sizeof(Header);
		size_t vertexBytes = header.vertexCount * sizeof(render::Mesh::Vertex);
		size_t indexBytes = header.indexCount * sizeof(uint32_t);
		totalSize += vertexBytes;
		totalSize += indexBytes;

		data.resize(totalSize);

		uint8_t* cursor = data.data();
		std::memcpy(cursor, &header, sizeof(Header));
		cursor += sizeof(Header);
		std::memcpy(cursor, meshPtr->GetVertex().data(), vertexBytes);
		cursor += vertexBytes;
		std::memcpy(cursor, meshPtr->GetIndices().data(), indexBytes);
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