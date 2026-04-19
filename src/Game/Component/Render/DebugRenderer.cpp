#include "Component/Render/DebugRenderer.h"

#include "Game/World.h"

#include "Render/Mesh.h"
#include "Render/Drawable.h"

#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
namespace sh::game
{
	DebugRenderer::DebugRenderer(game::GameObject& owner) :
		MeshRenderer(owner)
	{
		render::Material* const mat = static_cast<render::Material*>(core::SObjectManager::GetInstance()->GetSObject(core::UUID{"bbc4ef7ec45dce223297a224f8093f10"})); // ErrorMaterial
		assert(mat);
		SetMaterial(mat);

		position = Vec3{ 0.f, 0.f, 0.f };
		scale = Vec3{ 1.0f, 1.0f, 1.0f };
	}
	SH_GAME_API void DebugRenderer::SetPosition(const game::Vec3& pos)
	{
		position = pos;
	}
	SH_GAME_API void DebugRenderer::SetScale(const Vec3& scale)
	{
		this->scale = scale;
	}
	SH_GAME_API void DebugRenderer::SetRotation(const Vec3& rot)
	{
		quat = glm::quat{ glm::radians(glm::vec3{rot}) };
	}
	SH_GAME_API void DebugRenderer::SetQuat(const glm::quat& quat)
	{
		this->quat = quat;
	}
	SH_GAME_API void DebugRenderer::CreateDrawable(bool)
	{
		Super::CreateDrawable(false);
		if (!drawables.empty())
			drawables[0]->SetTopology(render::Mesh::Topology::Line);
	}
	SH_GAME_API void DebugRenderer::UpdateDrawable()
	{
		Super::UpdateDrawable();

		if (!drawables.empty())
			drawables[0]->SetModelMatrix(
				glm::translate(glm::mat4{ 1.0f }, 
				glm::vec3{ position }) * glm::mat4_cast(quat) * glm::scale(glm::mat4{ 1.0f }, glm::vec3{ scale }));
	}
}//namespace