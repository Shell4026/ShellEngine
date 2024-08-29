#include "Component/LineRenderer.h"

#include "GameObject.h"

#include <assert.h>

namespace sh::game
{
	LineRenderer::LineRenderer() :
		start(), end(0, 1, 0), mesh()
	{
		mesh.SetVertex({ start, end });
		mesh.SetIndices({ 0, 1 });
		mesh.SetTopology(render::Mesh::Topology::Line);

		this->SetMesh(mesh);
	}

	void LineRenderer::Awake()
	{
		mesh.Build(gameObject->world.renderer);

		render::Material* mat = gameObject->world.materials.GetResource("LineMat");
		assert(mat != nullptr);
		this->SetMaterial(*mat);

		Super::Awake();
	}

	void LineRenderer::Update()
	{
		Super::Update();
	}

#if SH_EDITOR
	void LineRenderer::OnPropertyChanged(const core::reflection::Property& prop)
	{
		if (std::strcmp(prop.GetName(), "start") == 0 || std::strcmp(prop.GetName(), "end") == 0)
		{
			mesh.SetVertex({ start, end });
			mesh.Build(gameObject->world.renderer);

			this->RebuildDrawables();
		}
	}
#endif
}//namespace