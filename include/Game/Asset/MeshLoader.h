#pragma once
#include "../Export.h"
#include "MeshAsset.h"

#include "Core/IAssetLoader.h"

namespace sh::render
{
	class IRenderContext;
}
namespace sh::game
{
	class MeshLoader : public core::IAssetLoader
	{
	public:
		SH_GAME_API MeshLoader(const render::IRenderContext& ctx);

		SH_GAME_API auto Load(const std::filesystem::path& filePath) const -> core::SObject* override;
		SH_GAME_API auto Load(const core::Asset& asset) const -> core::SObject* override;
		SH_GAME_API auto GetAssetName() const -> const char* override;
	private:
		const render::IRenderContext& ctx;
	};
}//namespace