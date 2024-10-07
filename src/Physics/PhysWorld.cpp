#include "PCH.h"
#include "PhysWorld.h"

namespace sh::phys
{
	void PhysWorld::Init()
	{
		world = physicsCommon.createPhysicsWorld();

	}
}//namespace