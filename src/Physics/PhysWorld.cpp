#include "PhysWorld.h"

#include "Core/Logger.h"

#include "reactphysics3d/reactphysics3d.h"
namespace sh::phys
{
	class GroundRaycastCallback : public reactphysics3d::RaycastCallback
	{
	public:
		bool hit = false;
		float hitFraction = 0.0f;
		reactphysics3d::Vector3 worldPoint;
		reactphysics3d::Vector3 worldNormal;

		// 첫 번째 히트에서 바로 멈추도록 false 반환
		virtual auto notifyRaycastHit(const reactphysics3d::RaycastInfo& info) -> reactphysics3d::decimal override
		{
			hit = true;
			hitFraction = info.hitFraction;
			worldPoint = info.worldPoint;
			worldNormal = info.worldNormal;
			return 0.0f;
		}
	};

	struct PhysWorld::Impl
	{
		reactphysics3d::PhysicsCommon physicsCommon;
		reactphysics3d::PhysicsWorld* world = nullptr;
	};
	SH_PHYS_API PhysWorld::PhysWorld()
	{
		impl = std::make_unique<Impl>();

		impl->world = impl->physicsCommon.createPhysicsWorld();
	}
	PhysWorld::PhysWorld(PhysWorld&& other) noexcept :
		impl(std::move(other.impl))
	{
	}
	SH_PHYS_API PhysWorld::~PhysWorld()
	{
		SH_INFO("~PhysWorld()");
	}

	SH_PHYS_API void PhysWorld::Clean()
	{
		impl->physicsCommon.destroyPhysicsWorld(impl->world);
	}

	SH_PHYS_API void PhysWorld::Update(float deltaTime)
	{
		impl->world->update(deltaTime);
	}
	SH_PHYS_API auto PhysWorld::RayCastHit(const Ray& ray) const -> bool
	{
		reactphysics3d::Vector3 start{ ray.origin.x, ray.origin.y, ray.origin.z };
		reactphysics3d::Vector3 dir{ ray.direction.x, ray.direction.y, ray.direction.z };
		dir.normalize();
		reactphysics3d::Ray reactPhysRay{ start , start + dir * ray.distance };
		GroundRaycastCallback callback;
		impl->world->raycast(reactPhysRay, &callback);

		return callback.hit;
	}
	SH_PHYS_API auto PhysWorld::GetContext() const -> void*
	{
		return &impl->physicsCommon;
	}
	SH_PHYS_API auto PhysWorld::GetNative() const -> void*
	{
		return impl->world;
	}
}//namespace