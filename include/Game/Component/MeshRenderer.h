#pragma once

#include "Component.h"

#include "Game/Export.h"
#include "Render/Mesh.h"

namespace sh::game
{
	class MeshRenderer : public Component
	{
		SCLASS(MeshRenderer)
	private:
		sh::render::Mesh* mesh;
	public:
		SH_GAME_API MeshRenderer(GameObject& owner);

		SH_GAME_API void SetMesh(sh::render::Mesh& mesh);
		SH_GAME_API auto GetMesh() const -> const sh::render::Mesh&;

		SH_GAME_API void Update() override;
	};
}