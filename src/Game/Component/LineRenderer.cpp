#include "Component/LineRenderer.h"
#include "GameObject.h"

#include "Core/Logger.h"

#include <assert.h>

namespace sh::game
{
	LineRenderer::LineRenderer() :
		start(0, 0, 0), end(0, 1, 0), mesh(),
		bUpdate(false)
	{
		mesh.SetVertex({ start, end });
		mesh.SetIndices({ 0, 1 });
		mesh.SetTopology(render::Mesh::Topology::Line);

		Super::SetMesh(mesh);
	}

	void LineRenderer::Awake()
	{
		mesh.Build(gameObject->world.renderer);

		render::Material* mat = gameObject->world.materials.GetResource("LineMat");
		assert(mat != nullptr);
		Super::SetMaterial(*mat);

		Super::Awake();
	}

	void LineRenderer::BeginUpdate()
	{
		if (!bUpdate)
			return;

		mesh.SetVertex({ this->start, this->end });
		mesh.Build(gameObject->world.renderer);
		this->RebuildDrawables();

		bUpdate = false;
	}
	void LineRenderer::Update()
	{
		Super::Update();
	}
	void LineRenderer::SetStart(const glm::vec3& start)
	{
		this->start = start;
		bUpdate = true;
	}
	void LineRenderer::SetEnd(const glm::vec3& start)
	{
		this->end = start;
		bUpdate = true;
	}

#if SH_EDITOR
	void LineRenderer::OnPropertyChanged(const core::reflection::Property& prop)
	{
		if (std::strcmp(prop.GetName(), "start") == 0 || std::strcmp(prop.GetName(), "end") == 0)
		{
			bUpdate = true;
		}
	}
#endif
}//namespace