#pragma once

#include "Export.h"

#include <vector>
#include <memory>
namespace sh::render
{
	class Shader;

	class ShaderBuilder
	{
	public:
		enum class shaderType
		{
			Vertex,
			Fragment
		};
	private:
		static int idCount;
	protected:
		std::vector<unsigned char> vertShaderData;
		std::vector<unsigned char> fragShaderData;
	protected:
		static auto GetNextId() -> int;
	public:
		SH_RENDER_API ShaderBuilder();
		SH_RENDER_API ShaderBuilder(const ShaderBuilder& other);
		SH_RENDER_API ShaderBuilder(ShaderBuilder&& other) noexcept;

		SH_RENDER_API void SetData(shaderType type, const std::vector<unsigned char>& data);
		SH_RENDER_API void SetData(shaderType type, std::vector<unsigned char>&& data);

		SH_RENDER_API virtual auto Build() -> std::unique_ptr<Shader> = 0;
		SH_RENDER_API void ClearData();
	};
}