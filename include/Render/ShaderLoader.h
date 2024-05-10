﻿#pragma once

#include "Export.h"

#include "Shader.h"

#include <vector>
#include <string_view>
#include <optional>
#include <memory>
#include <type_traits>

namespace sh::core { class FileLoader; }

namespace sh::render 
{
	class ShaderBuilder;

	class ShaderLoader {
	private:
		std::unique_ptr<sh::core::FileLoader> loader;

		ShaderBuilder* builder;
	public:
		SH_RENDER_API ShaderLoader(ShaderBuilder* builder);
		SH_RENDER_API ~ShaderLoader();

		SH_RENDER_API auto LoadShader(std::string_view vertexShader, std::string_view fragShader) -> std::unique_ptr<Shader>;
		template<typename T>
		auto LoadShader(std::string_view vertexShader, std::string_view fragShader) -> std::enable_if_t<std::is_base_of_v<Shader, T>, std::unique_ptr<T>>
		{
			std::unique_ptr<Shader> shader{ LoadShader(vertexShader, fragShader) };
			if (shader.get() == nullptr)
				return nullptr;

			T* shaderPtr = sh::core::reflection::Cast<T>(shader.get());
			if (shaderPtr == nullptr)
				return nullptr;

			shader.release();
			std::unique_ptr<T> newShader = std::unique_ptr<T>(shaderPtr);
			return newShader;
		}
	};
}