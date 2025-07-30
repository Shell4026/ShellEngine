#include "PCH.h"
#include "PhysWorld.h"

#include "Core/Logger.h"

namespace sh::phys
{
	SH_PHYS_API PhysWorld::PhysWorld() :
		physicsCommon()
	{
		world = physicsCommon.createPhysicsWorld();
	}
	PhysWorld::PhysWorld(PhysWorld&& other) noexcept :
		physicsCommon(),
		world(other.world)
	{
		other.world = nullptr;
	}
	SH_PHYS_API PhysWorld::~PhysWorld()
	{
		SH_INFO("~PhysWorld()");
	}

	SH_PHYS_API void PhysWorld::Clean()
	{
		physicsCommon.destroyPhysicsWorld(world);
	}

	SH_PHYS_API auto PhysWorld::GetContext() -> reactphysics3d::PhysicsCommon&
	{
		return physicsCommon;
	}
	SH_PHYS_API auto PhysWorld::GetWorld() const -> reactphysics3d::PhysicsWorld*
	{
		return world;
	}

	SH_PHYS_API void PhysWorld::Update(float deltaTime)
	{
		world->update(deltaTime);
	}

	SH_PHYS_API void PhysWorld::DestroyRigidBody(reactphysics3d::RigidBody* body)
	{
		world->destroyRigidBody(body);
	}
}//namespace