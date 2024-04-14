#pragma once

#include "Export.h"

#include <vector>
#include <memory>
namespace sh::render
{
	class Shader;

	class SH_RENDER_API ShaderBuilder
	{
	public:
		enum class shaderType
		{
			Vertex,
			Fragment
		};
	protected:
		std::vector<unsigned char> vertShaderData;
		std::vector<unsigned char> fragShaderData;
	public:
		void SetData(shaderType type, const std::vector<unsigned char>& data);
		void SetData(shaderType type, std::vector<unsigned char>&& data);

		virtual auto Build() -> std::unique_ptr<Shader> = 0;
		void ClearData();
	};
}