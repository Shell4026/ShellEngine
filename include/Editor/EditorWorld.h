#pragma once

#include "Export.h"

#include "Game/World.h"

namespace sh::editor
{
	class EditorWorld : public game::World
	{
	public:
		EditorWorld(render::Renderer& renderer, core::GC& gc, const game::ComponentModule& module);
		~EditorWorld();
	};
}