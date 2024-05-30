#pragma once

#include "Export.h"

#include "Core/Util.h"

#include "glm/vec3.hpp"
#include "glm/vec2.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

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