#include "PCH.h"
#include "Component/LineRenderer.h"
#include "Component/Camera.h"

#include "GameObject.h"

#include "Core/Logger.h"

#include <assert.h>

namespace sh::game
{
	SH_GAME_API LineRenderer::LineRenderer(GameObject& owner) :
		MeshRenderer(owner),

		start({ 0, 0, 0 }), end({ 0, 1, 0 }), mesh(),
		color({ 1.f, 0.f, 0.f, 1.f })
	{
		mesh.SetVertex({ start, end });
		mesh.SetIndices({ 0, 1 });
		mesh.SetTopology(render::Mesh::Topology::Line);
		mesh.lineWidth = 1.f;
		mesh.Build(*world.renderer.GetContext());
		Super::SetMesh(&mesh);

		mat = world.materials.GetResource("LineMaterial");
		assert(mat);
		Super::SetMaterial(mat);
	}

	SH_GAME_API void LineRenderer::Awake()
	{
		Super::Awake();
	}

	SH_GAME_API void LineRenderer::Update()
	{
		for (auto& [cam, drawable] : drawables)
		{
			if (!cam->active)
				continue;

			struct Points
			{
				alignas(16) glm::vec3 start;
				alignas(16) glm::vec3 end;
			} points{ start, end };

			struct Color
			{
				alignas(16) glm::vec4 color;
			} color{ this->color };

			drawable->SetUniformData(1, &points, render::IDrawable::Stage::Vertex);
			drawable->SetUniformData(2, &color, render::IDrawable::Stage::Fragment);
		}//drawables
		Super::Update();
	}
	SH_GAME_API void LineRenderer::SetStart(const Vec3& start)
	{
		this->start = start;

	}
	SH_GAME_API void LineRenderer::SetEnd(const Vec3& start)
	{
		this->end = start;
	}
	SH_GAME_API void LineRenderer::SetColor(const Vec4& color)
	{
		this->color = color;
	}

	SH_GAME_API auto LineRenderer::GetStart() const -> const Vec3&
	{
		return start;
	}
	SH_GAME_API auto LineRenderer::GetEnd() const -> const Vec3&
	{
		return end;
	}
	SH_GAME_API auto LineRenderer::GetColor() const -> const Vec4&
	{
		return color;
	}
}//namespace