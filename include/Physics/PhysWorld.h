#pragma once

#include "Export.h"

namespace sh::phys
{
	class PhysWorld
	{
	private:
		reactphysics3d::PhysicsCommon physicsCommon;
		reactphysics3d::PhysicsWorld* world;
	public:
		SH_PHYS_API void Init();
	};
}//namespace