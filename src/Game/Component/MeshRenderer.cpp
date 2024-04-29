#include "Component/MeshRenderer.h"
#include "Render/Renderer.h"

#include "GameObject.h"

namespace sh::game
{
	MeshRenderer::MeshRenderer(GameObject& owner) :
		Component(owner)
	{
	}

	void MeshRenderer::SetMesh(sh::render::Mesh& mesh)
	{
		this->mesh = &mesh;
	}

	auto MeshRenderer::GetMesh() const -> const sh::render::Mesh&
	{
		return *mesh;
	}

	void MeshRenderer::Update()
	{
		gameObject.world.renderer.PushDrawAble(mesh);
	}
}