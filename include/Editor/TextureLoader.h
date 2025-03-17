#pragma once

#include "Export.h"

#include "Render/Texture.h"

#include "External/stb/stb_image.h"

#include <string_view>

namespace sh::render
{
	class IRenderContext;
}
namespace sh::editor
{
	class TextureLoader
	{
	public:
		const render::IRenderContext& context;
	public:
		SH_EDITOR_API TextureLoader(const render::IRenderContext& context);
		SH_EDITOR_API ~TextureLoader() = default;
		SH_EDITOR_API auto Load(std::string_view filename, bool bGenerateMipmap = true) -> render::Texture*;
	};
}