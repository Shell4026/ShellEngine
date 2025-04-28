#pragma once
#include "Export.h"
#include "Component.h"

#include "Game/Vector.h"

#include "Core/SContainer.hpp"

namespace sh::game
{
	class RigidBody;
	class DebugRenderer;
	class BoxCollider : public Component
	{
		COMPONENT(BoxCollider)
		friend RigidBody;
	private:
		reactphysics3d::BoxShape* shape = nullptr;
		core::SSet<RigidBody*> rigidbodies;

		DebugRenderer* debugRenderer = nullptr;
		PROPERTY(bDisplayArea)
		bool bDisplayArea = false;

		PROPERTY(size)
		Vec3 size;
	public:
		SH_GAME_API BoxCollider(GameObject& owner);
		SH_GAME_API ~BoxCollider();

		SH_GAME_API void Destroy() override;
		SH_GAME_API void OnDestroy() override;

		SH_GAME_API auto operator*() const -> reactphysics3d::BoxShape*;

		SH_GAME_API void SetSize(const Vec3& size);
		SH_GAME_API auto GetSize() const -> const Vec3&;

		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& prop) override;

		SH_GAME_API void Update() override;

		SH_GAME_API void DisplayArea(bool bDisplay = true);
	};
}//namespace
