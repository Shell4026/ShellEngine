#include "Asset/MeshLoader.h"

#include <cstring>
namespace sh::game
{
	MeshLoader::MeshLoader(const render::IRenderContext& ctx) :
		ctx(ctx)
	{
	}
	SH_GAME_API auto MeshLoader::Load(const std::filesystem::path& filePath) -> core::SObject*
	{
		return nullptr;
	}
	SH_GAME_API auto MeshLoader::Load(const core::Asset& asset) -> core::SObject*
	{
		if (std::strcmp(asset.GetType(), GetAssetName()) != 0)
			return nullptr;

		const MeshAsset& meshAsset = static_cast<const MeshAsset&>(asset);
		const MeshAsset::Header header = meshAsset.GetHeader();
		const MeshAsset::MeshData data = meshAsset.GetData();

		std::vector<render::Mesh::Vertex> vertices(header.vertexCount);
		std::vector<uint32_t> indices(header.indexCount);

		const size_t vertexBytes = header.vertexCount * sizeof(render::Mesh::Vertex);
		const size_t indexBytes = header.indexCount * sizeof(uint32_t);

		const uint8_t* cursor = data.dataPtr;
		std::memcpy(vertices.data(), cursor, vertexBytes);
		cursor += vertexBytes;
		std::memcpy(indices.data(), cursor, indexBytes);

		auto mesh = core::SObject::Create<render::Mesh>();
		mesh->SetUUID(asset.GetAssetUUID());
		mesh->SetVertex(std::move(vertices));
		mesh->SetIndices(std::move(indices));
		mesh->Build(ctx);

		return mesh;
	}
	SH_GAME_API auto MeshLoader::GetAssetName() const -> const char*
	{
		return MeshAsset::ASSET_NAME;
	}
}//namespace