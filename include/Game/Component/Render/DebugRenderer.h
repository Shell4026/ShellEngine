#pragma once
#include "Game/Export.h"
#include "Game/Vector.h"
#include "MeshRenderer.h"

#include <glm/gtc/quaternion.hpp>
namespace sh::game
{
	class DebugRenderer : public MeshRenderer
	{
		COMPONENT(DebugRenderer)
	public:
		SH_GAME_API DebugRenderer(GameObject& owner);

		SH_GAME_API auto Serialize() const -> core::Json override { return core::Json{}; }

		SH_GAME_API void SetPosition(const Vec3& pos);
		SH_GAME_API void SetScale(const Vec3& scale);
		SH_GAME_API void SetRotation(const Vec3& rot);
		SH_GAME_API void SetQuat(const glm::quat& quat);

		SH_GAME_API auto GetPosition() const -> const Vec3& { return position; }
		SH_GAME_API auto GetScale() const -> const Vec3& { return scale; }
		SH_GAME_API auto GetQuat() const -> glm::quat { return quat; }
	protected:
		SH_GAME_API void CreateDrawable(bool) override;
		SH_GAME_API void UpdateDrawable() override;
	private:
		PROPERTY(position)
		Vec3 position;
		PROPERTY(scale)
		Vec3 scale;
		PROPERTY(quat)
		glm::quat quat;
	};
}//namespace