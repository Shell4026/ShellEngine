#include "PCH.h"
#include "Component/LineRenderer.h"
#include "GameObject.h"

#include "Core/Logger.h"

#include <assert.h>

namespace sh::game
{
	LineRenderer::LineRenderer(GameObject& owner) :
		MeshRenderer(owner),

		start(0, 0, 0), end(0, 1, 0), mesh(), mat(nullptr),
		bUpdate(false)
	{
		mesh.SetVertex({ start, end });
		mesh.SetIndices({ 0, 1 });
		mesh.SetTopology(render::Mesh::Topology::Line);

		Super::SetMesh(&mesh);
	}

	void LineRenderer::Awake()
	{
		mesh.Build(gameObject.world.renderer);

		mat = gameObject.world.materials.GetResource("LineMat");
		mat->SetVector("start", glm::vec4(start, 1.f));
		mat->SetVector("end", glm::vec4(end, 1.f));
		assert(mat != nullptr);
		Super::SetMaterial(mat);

		Super::Awake();
	}

	void LineRenderer::BeginUpdate()
	{
		if (!bUpdate)
			return;

		mat->SetVector("start", glm::vec4(start, 1.f));
		mat->SetVector("end", glm::vec4(end, 1.f));

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