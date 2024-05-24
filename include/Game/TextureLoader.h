#pragma once

#include "Export.h"

#include "Render/Texture.h"

#include "External/stb/stb_image.h"

#include <string_view>
#include <memory>

namespace sh::game
{
	class TextureLoader
	{
	public:
		SH_GAME_API auto Load(std::string_view filename) -> std::unique_ptr<render::Texture>;
	};
}