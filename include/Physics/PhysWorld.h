#pragma once

#include "Export.h"

namespace sh::phys
{
	class PhysWorld
	{
	private:
		reactphysics3d::PhysicsCommon physicsCommon;
		reactphysics3d::PhysicsWorld* world = nullptr;
		reactphysics3d::RigidBody* floor = nullptr;
	public:
		SH_PHYS_API PhysWorld();
		SH_PHYS_API ~PhysWorld();

		SH_PHYS_API void Clean();

		SH_PHYS_API auto GetContext() -> reactphysics3d::PhysicsCommon&;
		SH_PHYS_API auto GetWorld() const -> reactphysics3d::PhysicsWorld*;

		SH_PHYS_API void Update(float deltaTime);

		SH_PHYS_API void DestroyRigidBody(reactphysics3d::RigidBody* body);
	};
}//namespace