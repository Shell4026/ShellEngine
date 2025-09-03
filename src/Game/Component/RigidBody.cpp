#include "Component/RigidBody.h"

#include "GameObject.h"
#include "PhysWorld.h"

#include "Core/Logger.h"

#include "reactphysics3d/reactphysics3d.h"
namespace sh::game
{
	struct RigidBody::Impl
	{
		reactphysics3d::Collider* collider = nullptr;
		reactphysics3d::RigidBody* rigidbody = nullptr;
	};

	SH_GAME_API RigidBody::RigidBody(GameObject& owner) :
		Component(owner)
	{
		auto ctx = reinterpret_cast<reactphysics3d::PhysicsCommon*>(gameObject.world.GetPhysWorld()->GetContext());
		auto world = reinterpret_cast<reactphysics3d::PhysicsWorld*>(gameObject.world.GetPhysWorld()->GetNative());

		reactphysics3d::Vector3 physPos{ gameObject.transform->position.x, gameObject.transform->position.y, gameObject.transform->position.z };
		auto& quat = gameObject.transform->GetQuat();
		reactphysics3d::Quaternion physQuat{ quat.x, quat.y, quat.z, quat.w };
		reactphysics3d::Transform transform{ physPos, physQuat };

		impl = std::make_unique<Impl>();

		impl->rigidbody = world->createRigidBody(transform);
		impl->rigidbody->setType(reactphysics3d::BodyType::DYNAMIC);
		impl->rigidbody->enableGravity(bGravity);

		colliderDestroyListener.SetCallback(
			[&](const core::SObject* obj)
			{
				const Collider* colliderComponent = static_cast<const Collider*>(obj);
				if (impl->collider != nullptr)
				{
					if (impl->rigidbody != nullptr)
						impl->rigidbody->removeCollider(impl->collider);
					impl->collider = nullptr;
				}
			}
		);
	}
	SH_GAME_API RigidBody::~RigidBody()
	{
		SH_INFO("~RigidBody");
	}
	SH_GAME_API void RigidBody::Start()
	{
		const auto& objQuat = gameObject.transform->GetWorldQuat();
		const Vec3& objPos = gameObject.transform->GetWorldPosition();
		impl->rigidbody->setTransform(reactphysics3d::Transform{ {objPos.x, objPos.y, objPos.z}, reactphysics3d::Quaternion{objQuat.x, objQuat.y, objQuat.z, objQuat.w} });
		if (&collision->gameObject != &gameObject)
		{
			if (core::IsValid(collision))
			{
				const auto& pos = collision->gameObject.transform->position;
				const auto& quat = collision->gameObject.transform->GetQuat();
				impl->collider->setLocalToBodyTransform(reactphysics3d::Transform{ {pos.x, pos.y, pos.z}, {quat.x, quat.y, quat.z, quat.w} });
			}
		}

		prevPos = objPos;
		prevRot = objQuat;
		currPos = objPos;
		currRot = objQuat;
	}
	SH_GAME_API void RigidBody::OnDestroy()
	{
		if (impl->collider != nullptr)
			impl->rigidbody->removeCollider(impl->collider);

		auto world = reinterpret_cast<reactphysics3d::PhysicsWorld*>(gameObject.world.GetPhysWorld()->GetNative());
		world->destroyRigidBody(impl->rigidbody);
		impl->rigidbody = nullptr;

		Super::OnDestroy();
	}
	SH_GAME_API void RigidBody::BeginUpdate()
	{
		if (impl->collider != nullptr && !core::IsValid(collision))
			SetCollider(nullptr);

		if (&collision->gameObject != &gameObject)
		{
			if (core::IsValid(collision))
			{
				const auto& pos = collision->gameObject.transform->position;
				const auto& quat = collision->gameObject.transform->GetQuat();
				impl->collider->setLocalToBodyTransform(reactphysics3d::Transform{ {pos.x, pos.y, pos.z}, {quat.x, quat.y, quat.z, quat.w} });
			}
		}
	}
	SH_GAME_API void RigidBody::FixedUpdate()
	{
		prevPos = currPos;
		prevRot = currRot;

		auto& pos = impl->rigidbody->getTransform().getPosition();
		auto& quat = impl->rigidbody->getTransform().getOrientation();
		currPos = glm::vec3(pos.x, pos.y, pos.z);
		currRot = glm::quat{ quat.w, quat.x, quat.y, quat.z };
	}
	SH_GAME_API void RigidBody::Update()
	{
		if (!bKinematic)
		{
			float alpha = std::clamp(gameObject.world.fixedDeltaTime / gameObject.world.FIXED_TIME, 0.f, 1.f);
			glm::vec3 interpPos = glm::mix(prevPos, currPos, alpha);
			glm::quat interpRot = glm::slerp(prevRot, currRot, alpha);
			interpRot = glm::normalize(interpRot);

			gameObject.transform->SetWorldPosition(interpPos);
			gameObject.transform->SetWorldRotation(interpRot);
			gameObject.transform->UpdateMatrix();
		}
		else
		{
			gameObject.transform->SetWorldPosition(currPos);
			gameObject.transform->SetWorldRotation(currRot);
			gameObject.transform->UpdateMatrix();
		}
	}
	SH_GAME_API void RigidBody::LateUpdate()
	{
	}

	SH_GAME_API void RigidBody::SetKinematic(bool set)
	{
		bKinematic = set;
		if (bKinematic)
			impl->rigidbody->setType(reactphysics3d::BodyType::KINEMATIC);
		else
			impl->rigidbody->setType(reactphysics3d::BodyType::DYNAMIC);
	}
	SH_GAME_API void RigidBody::SetGravity(bool use)
	{
		bGravity = use;
		impl->rigidbody->enableGravity(bGravity);
	}
	SH_GAME_API void RigidBody::SetCollider(Collider* colliderComponent)
	{
		if (impl->rigidbody == nullptr)
			return;

		if (collision != nullptr)
			collision->onDestroy.UnRegister(colliderDestroyListener);

		collision = colliderComponent;

		if (collision != nullptr)
			collision->onDestroy.Register(colliderDestroyListener);

		if (impl->collider != nullptr)
		{
			impl->rigidbody->removeCollider(impl->collider);
			impl->collider = nullptr;
		}

		if (core::IsValid(collision))
		{
			auto shape = reinterpret_cast<reactphysics3d::CollisionShape*>(collision->GetNative());
			const auto& quat = colliderComponent->gameObject.transform->GetQuat();
			const Vec3& pos = colliderComponent->gameObject.transform->position;
			reactphysics3d::Transform transform{};
			// 리지드 바디랑 콜라이더랑 같은 오브젝트에 있으면 transform은 identity
			if (&colliderComponent->gameObject != &gameObject)
				transform = reactphysics3d::Transform{ { pos.x, pos.y, pos.z }, { quat.x,quat.y,quat.z,quat.w } };
			impl->collider = impl->rigidbody->addCollider(shape, transform);
			impl->collider->getMaterial().setBounciness(bouncy);
			//impl->rigidbody->getCollider(0)->set
		}
	}

	SH_GAME_API void RigidBody::SetMass(float mass)
	{
		impl->rigidbody->setMass(mass);
	}

	SH_GAME_API void RigidBody::SetLinearVelocity(const game::Vec3& v)
	{
		impl->rigidbody->setLinearVelocity({ v.x, v.y, v.z });
	}

	SH_GAME_API void RigidBody::SetAngularVelocity(const game::Vec3& v)
	{
		impl->rigidbody->setAngularVelocity({ v.x, v.y, v.z });
	}

	SH_GAME_API void RigidBody::SetLinearDamping(float damping)
	{
		impl->rigidbody->setLinearDamping(damping);
	}

	SH_GAME_API void RigidBody::SetAngularDamping(float damping)
	{
		impl->rigidbody->setAngularDamping(damping);
	}

	SH_GAME_API void RigidBody::AddWorldTorque(const game::Vec3& torque)
	{
		impl->rigidbody->applyWorldTorque({ torque.x, torque.y, torque.z });
	}

	SH_GAME_API void RigidBody::AddWorldForce(const game::Vec3& force)
	{
		impl->rigidbody->applyWorldForceAtCenterOfMass({ force.x, force.y, force.z });
	}

	SH_GAME_API void RigidBody::AddTorque(const game::Vec3& torque)
	{
		impl->rigidbody->applyLocalTorque({ torque.x, torque.y, torque.z });
	}

	SH_GAME_API void RigidBody::AddForce(const game::Vec3& force)
	{
		impl->rigidbody->applyLocalForceAtCenterOfMass({ force.x, force.y, force.z });
	}

	SH_GAME_API void RigidBody::SetAxisLock(const game::Vec3& dir)
	{
		axisLock = dir;
		axisLock.x = std::clamp(std::roundf(axisLock.x), 0.f, 1.f);
		axisLock.y = std::clamp(std::roundf(axisLock.y), 0.f, 1.f);
		axisLock.z = std::clamp(std::roundf(axisLock.z), 0.f, 1.f);

		// reactPhysics에선 0이 허용, 1이 잠금이기 때문에 반전 시켜야함.
		bool x = !static_cast<bool>(dir.x);
		bool y = !static_cast<bool>(dir.y);
		bool z = !static_cast<bool>(dir.z);
		impl->rigidbody->setAngularLockAxisFactor({ static_cast<float>(x), static_cast<float>(y), static_cast<float>(z) });
	}

	SH_GAME_API auto RigidBody::GetAxisLock() const -> const game::Vec3&
	{
		return axisLock;
	}

	SH_GAME_API void RigidBody::SetBouncy(float bouncy)
	{
		this->bouncy = std::clamp(bouncy, 0.f, 1.f);
		if (impl->collider != nullptr)
			impl->collider->getMaterial().setBounciness(bouncy);
	}

	SH_GAME_API auto RigidBody::GetBouncy() const -> float
	{
		return bouncy;
	}

	SH_GAME_API bool RigidBody::IsKinematic() const
	{
		return bKinematic;
	}
	SH_GAME_API bool RigidBody::IsGravityUse() const
	{
		return bGravity;
	}

	SH_GAME_API auto RigidBody::GetMass() const -> float
	{
		return impl->rigidbody->getMass();
	}

	SH_GAME_API auto RigidBody::GetLinearDamping() const -> float
	{
		return impl->rigidbody->getLinearDamping();
	}
	SH_GAME_API auto sh::game::RigidBody::GetAngularDamping() const -> float
	{
		return impl->rigidbody->getAngularDamping();
	}
	SH_GAME_API auto RigidBody::GetLinearVelocity() const -> game::Vec3
	{
		auto v = impl->rigidbody->getLinearVelocity();
		return game::Vec3{ v.x, v.y, v.z };
	}

	SH_GAME_API auto RigidBody::GetAngularVelocity() const -> game::Vec3
	{
		auto v = impl->rigidbody->getAngularVelocity();
		return game::Vec3{ v.x, v.y, v.z };
	}

	SH_GAME_API auto RigidBody::GetForce() const -> game::Vec3
	{
		auto& f = impl->rigidbody->getForce();
		return game::Vec3{ f.x, f.y, f.z };
	}

	SH_GAME_API void RigidBody::SetSleep()
	{
		impl->rigidbody->setIsSleeping(true);
	}

	SH_GAME_API void RigidBody::ResetPhysicsTransform()
	{
		const Vec3& objPos = gameObject.transform->position;
		auto& objQuat = gameObject.transform->GetQuat();
		
		impl->rigidbody->setTransform(reactphysics3d::Transform{ {objPos.x, objPos.y, objPos.z}, reactphysics3d::Quaternion{objQuat.x, objQuat.y, objQuat.z, objQuat.w} });

		prevPos = objPos;
		prevRot = objQuat;
		currPos = objPos;
		currRot = objQuat;
	}

	SH_GAME_API void RigidBody::OnPropertyChanged(const core::reflection::Property& prop)
	{
		if (prop.GetName() == core::Util::ConstexprHash("collision"))
		{
			SetCollider(collision);
		}
		else if (prop.GetName() == core::Util::ConstexprHash("bGravity"))
		{
			SetGravity(bGravity);
		}
		else if (prop.GetName() == core::Util::ConstexprHash("bKinematic"))
		{
			SetKinematic(bKinematic);
		}
		else if (prop.GetName() == core::Util::ConstexprHash("mass"))
		{
			SetMass(mass);
		}
		else if (prop.GetName() == core::Util::ConstexprHash("linearDamping"))
		{
			SetLinearDamping(linearDamping);
		}
		else if (prop.GetName() == core::Util::ConstexprHash("angularDamping"))
		{
			SetAngularDamping(angularDamping);
		}
		else if (prop.GetName() == core::Util::ConstexprHash("axisLock"))
		{
			SetAxisLock(axisLock);
		}
		else if (prop.GetName() == core::Util::ConstexprHash("bouncy"))
		{
			SetBouncy(bouncy);
		}
	}
}//namespace