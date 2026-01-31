#pragma once
#include "../Export.h"

#include "Core/Asset.h"
#include "Core/SContainer.hpp"

#include "Render/Mesh.h"
namespace sh::game
{
	class MeshAsset : public core::Asset
	{
		SASSET(MeshAsset, "mesh")
	public:
		struct Header
		{
			uint64_t vertexCount;
			uint64_t indexCount;
		};
		struct MeshData
		{
			const uint8_t* dataPtr;
			std::size_t size;
		};
	public:
		SH_GAME_API MeshAsset();
		SH_GAME_API MeshAsset(const render::Mesh& mesh);

		SH_GAME_API void SetAsset(const core::SObject& obj) override;

		SH_GAME_API auto GetHeader() const -> const Header&;
		SH_GAME_API auto GetData() const -> MeshData;
	protected:
		SH_GAME_API void SetAssetData() const override;
		SH_GAME_API auto ParseAssetData() -> bool override;
	public:
		constexpr static const char* ASSET_NAME = "mesh";
	private:
		core::SObjWeakPtr<const render::Mesh> meshPtr;

		Header header;
	};
}//namespace