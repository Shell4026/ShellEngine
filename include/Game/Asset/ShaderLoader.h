#pragma once
#include "../Export.h"

#include "Core/Asset.h"
#include "Core/IAssetLoader.h"

#include "Render/Shader.h"

#include <filesystem>

namespace sh::render
{
	class ShaderPassBuilder;
}

namespace sh::game 
{
	class ShaderLoader : public core::IAssetLoader
	{
	private:
		static constexpr const char* ASSET_NAME = "shad";

		std::filesystem::path cachePath;
		render::ShaderPassBuilder* passBuilder;
	public:
		SH_GAME_API ShaderLoader(render::ShaderPassBuilder* builder);
		SH_GAME_API ~ShaderLoader();

		SH_GAME_API void SetCachePath(const std::filesystem::path& path);
		SH_GAME_API auto GetCachePath() const -> const std::filesystem::path&;

		SH_GAME_API auto Load(const std::filesystem::path& filePath) -> core::SObject* override;
		SH_GAME_API auto Load(const core::Asset& asset) -> core::SObject* override;
		SH_GAME_API auto GetAssetName() const -> const char* override;
	};
}