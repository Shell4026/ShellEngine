#pragma once
#include "../Export.h"
#include "../Vector.h"
#include "MeshRenderer.h"
namespace sh::game
{
	class DebugRenderer : public MeshRenderer
	{
		COMPONENT(DebugRenderer)
	private:
		PROPERTY(position)
		Vec3 position;
		PROPERTY(scale)
		Vec3 scale;
		PROPERTY(rotation)
		Vec3 rotation;
	protected:
		SH_GAME_API void CreateDrawable() override;
		SH_GAME_API void UpdateDrawable() override;
	public:
		SH_GAME_API DebugRenderer(GameObject& owner);

		SH_GAME_API void SetPosition(const Vec3& pos);
		SH_GAME_API auto GetPosition() const -> const Vec3&;
		SH_GAME_API void SetScale(const Vec3& scale);
		SH_GAME_API auto GetScale() const -> const Vec3&;
		SH_GAME_API void SetRotation(const Vec3& rot);
		SH_GAME_API auto GetRotation() const -> const Vec3&;

		SH_GAME_API auto Serialize() const -> core::Json override;
	};
}//namespace