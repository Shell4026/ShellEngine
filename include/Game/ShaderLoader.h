#pragma once
#include "Export.h"

#include "Render/Shader.h"

#include <filesystem>

namespace sh::render
{
	class ShaderPassBuilder;
}

namespace sh::game 
{
	class ShaderLoader 
	{
	private:
		render::ShaderPassBuilder* builder;
	public:
		SH_GAME_API ShaderLoader(render::ShaderPassBuilder* builder);
		SH_GAME_API ~ShaderLoader();

		SH_GAME_API auto LoadShader(const std::filesystem::path& path) -> render::Shader*;
	};
}