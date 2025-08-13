#pragma once
#include "Export.h"

#include "Core/IAssetLoader.h"

#include "Render/ShaderPass.h"

#include <filesystem>

namespace sh::render
{
	class IRenderContext;
	class Material;
}

namespace sh::game
{
	class MaterialLoader : public core::IAssetLoader
	{
	public:
		SH_GAME_API MaterialLoader(const render::IRenderContext& context);

		SH_GAME_API auto Load(const std::filesystem::path& filePath) -> core::SObject* override;
		SH_GAME_API auto Load(const core::Asset& asset) -> core::SObject* override;
		SH_GAME_API auto GetAssetName() const -> const char* override;
	private:
		const render::IRenderContext& context;
	};
}//namespace