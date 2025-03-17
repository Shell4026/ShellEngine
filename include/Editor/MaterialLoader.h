#pragma once
#include "Export.h"

#include "Render/ShaderPass.h"

namespace sh::render
{
	class IRenderContext;
	class Material;
}

namespace sh::editor
{
	class MaterialLoader
	{
	private:
		const render::IRenderContext& context;
	public:
		SH_EDITOR_API MaterialLoader(const render::IRenderContext& context);

		SH_EDITOR_API virtual auto Load(std::string_view filename) -> render::Material*;
	};
}//namespace