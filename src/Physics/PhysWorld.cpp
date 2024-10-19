#include "PCH.h"
#include "PhysWorld.h"

#include "Core/Logger.h"

namespace sh::phys
{
	SH_PHYS_API PhysWorld::PhysWorld() :
		physicsCommon()
	{
		world = physicsCommon.createPhysicsWorld();
		floor = world->createRigidBody(reactphysics3d::Transform::identity());
		floor->setType(reactphysics3d::BodyType::STATIC);
		
		auto collider = physicsCommon.createBoxShape(reactphysics3d::Vector3(10, 0.01, 10));
		floor->addCollider(collider, reactphysics3d::Transform::identity());
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