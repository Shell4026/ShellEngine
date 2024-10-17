#include "Game/PCH.h"
#include "EditorWorld.h"

#include "Game/GameObject.h"
#include "Game/Component/MeshRenderer.h"

namespace sh::editor
{
	SH_EDITOR_API EditorWorld::EditorWorld(render::Renderer& renderer, const game::ComponentModule& module) :
		World(renderer, module)
	{
	}

	SH_EDITOR_API EditorWorld::~EditorWorld()
	{
	}

	SH_EDITOR_API void EditorWorld::SetSelectedObject(game::GameObject* gameObject)
	{
		selected = gameObject;
	}
	SH_EDITOR_API auto EditorWorld::GetSelectedObject() const -> game::GameObject*
	{
		return selected;
	}

	void EditorWorld::Start()
	{
		auto grid = this->AddGameObject("Grid");
		auto meshRenderer = grid->AddComponent<game::MeshRenderer>();
		meshRenderer->SetMesh(*this->meshes.GetResource("GridMesh"));
		meshRenderer->SetMaterial(*this->materials.GetResource("GridMaterial"));

		Super::Start();
	}
}