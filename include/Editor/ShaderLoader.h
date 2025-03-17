#pragma once
#include "Export.h"

#include "Render/Shader.h"

#include <filesystem>

namespace sh::render
{
	class ShaderPassBuilder;
}

namespace sh::editor 
{
	class ShaderLoader 
	{
	private:
		std::filesystem::path cachePath;
		render::ShaderPassBuilder* passBuilder;
	public:
		SH_EDITOR_API ShaderLoader(render::ShaderPassBuilder* builder);
		SH_EDITOR_API ~ShaderLoader();

		SH_EDITOR_API void SetCachePath(const std::filesystem::path& path);
		SH_EDITOR_API auto GetCachePath() const -> const std::filesystem::path&;
		SH_EDITOR_API auto LoadShader(const std::filesystem::path& path) -> render::Shader*;
	};
}