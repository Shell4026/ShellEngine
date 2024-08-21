#pragma once

#include "Export.h"

#include "Game/World.h"

namespace sh::editor
{
	class EditorWorld : public game::World
	{
	public:
		EditorWorld(render::Renderer&, const game::ComponentModule& module);
		~EditorWorld();
	};
}