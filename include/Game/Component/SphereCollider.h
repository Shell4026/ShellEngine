#pragma once
#include "Export.h"
#include "Collider.h"

#include "Game/Vector.h"

#include "Core/SContainer.hpp"

#include "reactphysics3d/reactphysics3d.h"
namespace sh::game
{
	class RigidBody;
	class DebugRenderer;
	class ShpereCollider : public Collider
	{
		COMPONENT(ShpereCollider)
		friend RigidBody;
	private:
		reactphysics3d::SphereShape* shape = nullptr;

		DebugRenderer* debugRenderer = nullptr;
		PROPERTY(bDisplayArea)
		bool bDisplayArea = false;

		PROPERTY(radius)
		float radius;
	public:
		SH_GAME_API ShpereCollider(GameObject& owner);
		SH_GAME_API ~ShpereCollider();

		SH_GAME_API void OnDestroy() override;

		SH_GAME_API auto GetCollisionShape() const -> reactphysics3d::CollisionShape*;

		SH_GAME_API void SetRadius(float r);
		SH_GAME_API auto GetRadius() const -> float;

		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& prop) override;

		SH_GAME_API void Update() override;

		SH_GAME_API void DisplayArea(bool bDisplay = true);
	};
}//namespace
