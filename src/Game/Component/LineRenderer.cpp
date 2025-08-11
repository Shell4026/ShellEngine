#include "Component/LineRenderer.h"
#include "Component/Camera.h"

#include "Render/Mesh.h"

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
		mesh.SetVertex({ { start, end } });
		mesh.SetIndices({ 0, 1 });
		mesh.SetTopology(render::Mesh::Topology::Line);
		mesh.lineWidth = 1.f;
		mesh.Build(*world.renderer.GetContext());
		Super::SetMesh(&mesh);

		mat = static_cast<render::Material*>(core::SObjectManager::GetInstance()->GetSObject(core::UUID{"bbc4ef7ec45dce223297a224f8093f11"}));
		assert(mat);
		Super::SetMaterial(mat);

		SetMaterialPropertyBlock(SObject::Create<render::MaterialPropertyBlock>());
	}

	SH_GAME_API void LineRenderer::Awake()
	{
		Super::Awake();
	}

	SH_GAME_API void LineRenderer::Update()
	{
		auto propertyBlock = GetMaterialPropertyBlock();
		propertyBlock->SetProperty("start", glm::vec3{ start });
		propertyBlock->SetProperty("end", glm::vec3{ end });
		propertyBlock->SetProperty("color", glm::vec4{ color });
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