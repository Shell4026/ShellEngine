#include "EditorWorld.h"

namespace sh::editor
{
	EditorWorld::EditorWorld(render::Renderer& renderer, core::GC& gc, const game::ComponentModule& module) :
		World(renderer, gc, module)
	{
	}

	EditorWorld::~EditorWorld()
	{
	}
}