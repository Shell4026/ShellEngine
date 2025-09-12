#include "PhysWorld.h"

#include "Core/Logger.h"

#include "reactphysics3d/reactphysics3d.h"
namespace sh::phys
{
	class GroundRaycastCallback : public reactphysics3d::RaycastCallback
	{
	public:
		struct HitInfo
		{
			float hitFraction = 0.0f;
			reactphysics3d::Vector3 worldPoint;
			reactphysics3d::Vector3 worldNormal;
			void* rigidBodyHandle = nullptr;
		};
		bool hit = false;
		std::vector<HitPoint> infos;

		// 모든 히트 수집
		virtual auto notifyRaycastHit(const reactphysics3d::RaycastInfo& info) -> reactphysics3d::decimal override
		{
			hit = true;
			HitPoint hit{};
			hit.fraction = info.hitFraction;
			hit.hitNormal = { info.worldNormal.x, info.worldNormal.y, info.worldNormal.z };
			hit.hitPoint = { info.worldPoint.x, info.worldPoint.y, info.worldPoint.z };
			hit.rigidBodyHandle = info.collider->getBody();
			infos.push_back(hit);
			return 1.0f;
		}
		
	};
	class GroundRaycastCallbackOnce : public reactphysics3d::RaycastCallback
	{
	public:
		bool hit = false;
		// 첫 히트 시 종료
		virtual auto notifyRaycastHit(const reactphysics3d::RaycastInfo& info) -> reactphysics3d::decimal override
		{
			hit = true;
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
	SH_PHYS_API auto PhysWorld::RayCastHit(const Ray& ray, Tagbit allowedTag) const -> bool
	{
		reactphysics3d::Vector3 start{ ray.origin.x, ray.origin.y, ray.origin.z };
		reactphysics3d::Vector3 dir{ ray.direction.x, ray.direction.y, ray.direction.z };
		dir.normalize();
		reactphysics3d::Ray reactPhysRay{ start , start + dir * ray.distance };
		GroundRaycastCallbackOnce callback;
		impl->world->raycast(reactPhysRay, &callback, allowedTag);

		return callback.hit;
	}
	SH_PHYS_API auto PhysWorld::RayCast(const Ray& ray, Tagbit allowedTag) const -> std::vector<HitPoint>
	{
		reactphysics3d::Vector3 start{ ray.origin.x, ray.origin.y, ray.origin.z };
		reactphysics3d::Vector3 dir{ ray.direction.x, ray.direction.y, ray.direction.z };
		dir.normalize();
		reactphysics3d::Ray reactPhysRay{ start , start + dir * ray.distance };
		GroundRaycastCallback callback;
		impl->world->raycast(reactPhysRay, &callback, allowedTag);

		if (!callback.hit)
			return {};

		std::vector<HitPoint> hits = std::move(callback.infos);
		std::sort(hits.begin(), hits.end(),
			[](const HitPoint& left, const HitPoint& right)
			{
				return left.fraction < right.fraction;
			}
		);
		return hits;
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