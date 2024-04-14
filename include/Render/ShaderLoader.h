#pragma once

#include "Export.h"

#include "Shader.h"

#include <vector>
#include <string_view>
#include <optional>
#include <memory>

namespace sh::core { class FileLoader; }

namespace sh::render 
{
	class ShaderBuilder;

	class SH_RENDER_API ShaderLoader {
	private:
		std::unique_ptr<sh::core::FileLoader> loader;

		ShaderBuilder* builder;
	public:
		ShaderLoader(ShaderBuilder* builder);
		~ShaderLoader();

		auto LoadShader(std::string_view vertexShader, std::string_view fragShader)->std::unique_ptr<Shader>;
	};
}