#pragma once

#include "Export.h"

#include "Render/Texture.h"

#include "External/stb/stb_image.h"

#include <string_view>

namespace sh::render
{
	class Renderer;
}
namespace sh::game
{
	class TextureLoader
	{
	public:
		render::Renderer& renderer;
	public:
		SH_GAME_API TextureLoader(render::Renderer& renderer);
		SH_GAME_API ~TextureLoader() = default;
		SH_GAME_API auto Load(std::string_view filename, bool bGenerateMipmap = true) -> render::Texture*;
	};
}