#pragma once

#include "Export.h"

#include "Render/Texture.h"

#include "External/stb/stb_image.h"

#include <string_view>
#include <memory>

namespace sh::render
{
	class Renderer;
}
namespace sh::game
{
	class TextureLoader
	{
	public:
		const render::Renderer& renderer;
	public:
		SH_GAME_API TextureLoader(const render::Renderer& renderer);
		SH_GAME_API ~TextureLoader() = default;
		SH_GAME_API auto Load(std::string_view filename) -> std::unique_ptr<render::Texture>;
	};
}