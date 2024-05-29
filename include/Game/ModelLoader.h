#pragma once

#include "Export.h"

#include <memory>
#include <string_view>

namespace sh::render
{
	class Renderer;
	class Mesh;
}
namespace sh::game
{
	class ModelLoader
	{
	public:
		const render::Renderer& renderer;
	public:
		SH_GAME_API ModelLoader(const render::Renderer& renderer);

		SH_GAME_API auto Load(std::string_view filename) -> std::unique_ptr<render::Mesh>;
	};
}