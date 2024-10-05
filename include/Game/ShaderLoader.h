#pragma once

#include "Export.h"

#include "Render/Shader.h"

#include <vector>
#include <string_view>
#include <optional>
#include <memory>
#include <type_traits>

namespace sh::core { class FileLoader; }

namespace sh::game 
{
	class ShaderBuilder;

	class ShaderLoader {
	private:
		std::unique_ptr<sh::core::FileLoader> loader;

		ShaderBuilder* builder;
	public:
		SH_GAME_API ShaderLoader(ShaderBuilder* builder);
		SH_GAME_API ~ShaderLoader();

		SH_GAME_API auto LoadShader(std::string_view vertexShader, std::string_view fragShader) -> render::Shader*;
		template<typename T>
		auto LoadShader(std::string_view vertexShader, std::string_view fragShader) -> std::enable_if_t<std::is_base_of_v<render::Shader, T>, T*>
		{
			render::Shader* shader{ LoadShader(vertexShader, fragShader) };
			if (shader == nullptr)
				return nullptr;

			T* shaderPtr = sh::core::reflection::Cast<T>(shader);
			if (shaderPtr == nullptr)
				return nullptr;

			return shaderPtr;
		}
	};
}