#pragma once
#include "../Export.h"

#include "Core/Asset.h"
#include "Core/IAssetLoader.h"

#include "Render/ComputeShader.h"

#include <filesystem>

namespace sh::render
{
	class IRenderContext;
}
namespace sh::game
{
	
	class ComputeShaderLoader : public core::IAssetLoader
	{
	public:
		SH_GAME_API ComputeShaderLoader(const render::IRenderContext& ctx);
		SH_GAME_API ~ComputeShaderLoader();

		SH_GAME_API void SetCachePath(const std::filesystem::path& path) { cachePath = path; }
		
		SH_GAME_API auto Load(const std::filesystem::path& filePath) const -> core::SObject* override;
		SH_GAME_API auto Load(const core::Asset& asset) const -> core::SObject* override;

		SH_GAME_API auto GetCachePath() const -> const std::filesystem::path& { return cachePath; }
		SH_GAME_API auto GetAssetName() const -> const char* override { return ASSET_NAME; }
	private:
		static constexpr const char* ASSET_NAME = "comp";
		const render::IRenderContext& ctx;

		std::filesystem::path cachePath;
	};
}//namespace
