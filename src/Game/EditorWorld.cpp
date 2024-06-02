#include "EditorWorld.h"

namespace sh::game
{
	EditorWorld::EditorWorld(render::Renderer& renderer, core::GC& gc) :
		World(renderer, gc)
	{
	}

	EditorWorld::~EditorWorld()
	{
	}
}