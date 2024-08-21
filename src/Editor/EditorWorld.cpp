#include "EditorWorld.h"

namespace sh::editor
{
	EditorWorld::EditorWorld(render::Renderer& renderer, const game::ComponentModule& module) :
		World(renderer, module)
	{
	}

	EditorWorld::~EditorWorld()
	{
	}
}