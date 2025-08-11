#include "Component/DebugRenderer.h"

#include "Game/World.h"

#include "Render/Mesh.h"

#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
namespace sh::game
{
	SH_GAME_API void DebugRenderer::CreateDrawable()
	{
		Super::CreateDrawable();
		drawable->SetTopology(render::Mesh::Topology::Line);
	}
	SH_GAME_API void DebugRenderer::UpdateDrawable()
	{
		Super::UpdateDrawable();
		
		glm::quat quat = glm::quat{ glm::radians(glm::vec3{ rotation }) };
		drawable->SetModelMatrix(glm::translate(glm::mat4{ 1.0f }, glm::vec3{ position }) * glm::mat4_cast(quat) * glm::scale(glm::mat4{ 1.0f }, glm::vec3{ scale }));
	}
	DebugRenderer::DebugRenderer(game::GameObject& owner) :
		MeshRenderer(owner)
	{
		mat = world.materials.GetResource("WireframeMat");
		assert(mat);
		position = Vec3{ 0.f, 0.f, 0.f };
		scale = Vec3{ 1.0f, 1.0f, 1.0f };
		rotation = Vec3{ 0.f, 0.f, 0.f };
	}
	SH_GAME_API void DebugRenderer::SetPosition(const game::Vec3& pos)
	{
		position = pos;
	}
	SH_GAME_API auto DebugRenderer::GetPosition() const -> const game::Vec3&
	{
		return position;
	}
	SH_GAME_API void DebugRenderer::SetScale(const Vec3& scale)
	{
		this->scale = scale;
	}
	SH_GAME_API auto DebugRenderer::GetScale() const -> const Vec3&
	{
		return scale;
	}
	SH_GAME_API void DebugRenderer::SetRotation(const Vec3& rot)
	{
		rotation = rot;
	}
	SH_GAME_API auto DebugRenderer::GetRotation() const -> const Vec3&
	{
		return rotation;
	}
	SH_GAME_API auto DebugRenderer::Serialize() const -> core::Json
	{
		return core::Json{};
	}
}//namespace