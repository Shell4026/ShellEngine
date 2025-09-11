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
		void* rigidBodyHandle = nullptr;

		// 첫 번째 히트에서 바로 멈추도록 false 반환
		virtual auto notifyRaycastHit(const reactphysics3d::RaycastInfo& info) -> reactphysics3d::decimal override
		{
			hit = true;
			hitFraction = info.hitFraction;
			worldPoint = info.worldPoint;
			worldNormal = info.worldNormal;
			rigidBodyHandle = info.collider->getBody();
			return 0.0f;
		}
		
	};
	class CustomEventListener : public reactphysics3d::EventListener
	{
	public:
		virtual void onContact(const CollisionCallback::CallbackData& callbackData) override
		{
			if (bus == nullptr)
				return;

			for (uint32_t i = 0; i < callbackData.getNbContactPairs(); i++)
			{
				const auto& pair = callbackData.getContactPair(i);
				auto type = pair.getEventType();

				PhysWorld::PhysicsEvent evt{};
				evt.rigidBody1Handle = pair.getBody1();
				evt.rigidBody2Handle = pair.getBody2();

				if (type == reactphysics3d::CollisionCallback::ContactPair::EventType::ContactStart)
					evt.type = PhysWorld::PhysicsEvent::Type::CollisionEnter;
				else if (type == reactphysics3d::CollisionCallback::ContactPair::EventType::ContactStay)
					evt.type = PhysWorld::PhysicsEvent::Type::CollisionStay;
				else if (type == reactphysics3d::CollisionCallback::ContactPair::EventType::ContactExit)
					evt.type = PhysWorld::PhysicsEvent::Type::CollisionExit;

				bus->Publish(evt);
			}
		}
	public:
		core::EventBus* bus;
	};

	struct PhysWorld::Impl
	{
		reactphysics3d::PhysicsCommon physicsCommon;
		reactphysics3d::PhysicsWorld* world = nullptr;
		CustomEventListener eventListener;
	};
	SH_PHYS_API PhysWorld::PhysWorld()
	{
		impl = std::make_unique<Impl>();

		impl->world = impl->physicsCommon.createPhysicsWorld();

		impl->eventListener.bus = &bus;
		impl->world->setEventListener(&impl->eventListener);
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
	SH_PHYS_API auto PhysWorld::RayCast(const Ray& ray) const -> std::optional<HitPoint>
	{
		reactphysics3d::Vector3 start{ ray.origin.x, ray.origin.y, ray.origin.z };
		reactphysics3d::Vector3 dir{ ray.direction.x, ray.direction.y, ray.direction.z };
		dir.normalize();
		reactphysics3d::Ray reactPhysRay{ start , start + dir * ray.distance };
		GroundRaycastCallback callback;
		impl->world->raycast(reactPhysRay, &callback);

		if (!callback.hit)
			return {};

		HitPoint hit{};
		hit.hitPoint = { callback.worldPoint.x, callback.worldPoint.y, callback.worldPoint.z };
		hit.hitNormal = { callback.worldNormal.x, callback.worldNormal.y, callback.worldNormal.z };
		hit.rigidBodyHandle = callback.rigidBodyHandle;

		return hit;
	}
	SH_PHYS_API auto PhysWorld::GetContext() const -> ContextHandle
	{
		return &impl->physicsCommon;
	}
	SH_PHYS_API auto PhysWorld::GetNative() const -> PhysicsWorldHandle
	{
		return impl->world;
	}
	SH_PHYS_API void PhysWorld::SetGravity(const glm::vec3& gravity)
	{
		impl->world->setGravity({ gravity.x,gravity.y,gravity.z });
	}
	SH_PHYS_API auto PhysWorld::GetGravity() const -> glm::vec3
	{
		auto g = impl->world->getGravity();
		return glm::vec3{ g.x, g.y, g.z };
	}
}//namespace