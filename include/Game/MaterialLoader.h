#pragma once
#include "Export.h"

#include "Render/Shader.h"

namespace sh::render
{
	class Renderer;
	class Material;
}

namespace sh::game
{
	class MaterialLoader
	{
	private:
		const render::Renderer& renderer;
	private:
		static void SetDefaultProperty(render::Material* mat, const render::Shader::UniformData& uniformData);
	public:
		SH_GAME_API MaterialLoader(const render::Renderer& renderer);

		SH_GAME_API virtual auto Load(std::string_view filename) -> render::Material*;
	};
}//namespace