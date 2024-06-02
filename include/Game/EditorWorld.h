#pragma once

#include "Export.h"

#include "World.h"

namespace sh::game
{
	class EditorWorld : public World
	{
	public:
		EditorWorld(render::Renderer& renderer, core::GC& gc);
		~EditorWorld();
	};
}