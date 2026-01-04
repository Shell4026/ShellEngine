#include "Component/RigidBody.h"

#include "GameObject.h"

#include "Core/Logger.h"

#include "reactphysics3d/reactphysics3d.h"

#include <cstdint>
namespace sh::game
{
	std::unordered_map<RigidBody::RigidBodyHandle, RigidBody*> RigidBody::nativeMap{};

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

		nativeMap.insert({ impl->rigidbody, this });

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

		physEventSubscriber.SetCallback(
			[this](const phys::PhysWorld::PhysicsEvent& evt)
			{
				if (evt.rigidBody1Handle == impl->rigidbody || evt.rigidBody2Handle == impl->rigidbody)
				{
					RigidBodyHandle otherHandle = (evt.rigidBody1Handle == impl->rigidbody) ? evt.rigidBody2Handle : evt.rigidBody1Handle;
					auto it = nativeMap.find(otherHandle);
					if (it == nativeMap.end()) // 일어날 수가 있나?
						return;
					Collider* collider = it->second->GetCollider();
					if (!core::IsValid(collider))
						return;

					if (evt.type == phys::PhysWorld::PhysicsEvent::Type::CollisionEnter || evt.type == phys::PhysWorld::PhysicsEvent::Type::TriggerEnter)
					{
						gameObject.OnCollisionEnter(*collider);
						hitCollidersCache.insert(collider);
					}
					else if (evt.type == phys::PhysWorld::PhysicsEvent::Type::CollisionExit || evt.type == phys::PhysWorld::PhysicsEvent::Type::TriggerExit)
					{
						gameObject.OnCollisionExit(*collider);
						hitCollidersCache.erase(collider);
					}
				}
			}
		);

		owner.world.GetPhysWorld()->bus.Subscribe(physEventSubscriber);
	}
	SH_GAME_API RigidBody::~RigidBody()
	{
		SH_INFO("~RigidBody");
	}
	SH_GAME_API void RigidBody::Awake()
	{
		impl->rigidbody->setTransform(reactphysics3d::Transform::identity());
		//impl->rigidbody->setLinearVelocity({ 0.f, 0.f, 0.f });
	}
	SH_GAME_API void RigidBody::Start()
	{
		ResetPhysicsTransform();
	}
	SH_GAME_API void RigidBody::OnEnable()
	{
		impl->rigidbody->setIsActive(true);
	}
	SH_GAME_API void RigidBody::OnDisable()
	{
		impl->rigidbody->setIsActive(false);
		for (auto hitCollider : hitCollidersCache)
		{
			if (!core::IsValid(hitCollider))
				continue;
			gameObject.OnCollisionExit(*hitCollider);
			for (auto rb : hitCollider->GetRigidbodies())
			{
				auto it = rb->hitCollidersCache.find(collision);
				if (it != rb->hitCollidersCache.end())
				{
					rb->hitCollidersCache.erase(it);
					rb->gameObject.OnCollisionExit(*collision);
				}
			}
		}
		hitCollidersCache.clear();
	}
	SH_GAME_API void RigidBody::OnDestroy()
	{
		nativeMap.erase(impl->rigidbody);

		if (impl->collider != nullptr)
		{
			auto& handles = collision->handles;
			auto it = std::find(handles.begin(), handles.end(), Collider::Handle{ this, impl->collider });
			if (it != handles.end())
				handles.erase(it);

			impl->rigidbody->removeCollider(impl->collider);
		}

		auto world = reinterpret_cast<reactphysics3d::PhysicsWorld*>(gameObject.world.GetPhysWorld()->GetNative());
		world->destroyRigidBody(impl->rigidbody);
		impl->rigidbody = nullptr;

		Super::OnDestroy();
	}
	SH_GAME_API void RigidBody::BeginUpdate()
	{
		// collision이 유효하지 않게 되면 콜라이더 재설정
		if (impl->collider != nullptr && !core::IsValid(collision))
			SetCollider(nullptr);
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
		Interpolate();
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
	SH_GAME_API void RigidBody::SetUsingGravity(bool use)
	{
		bGravity = use;
		impl->rigidbody->enableGravity(bGravity);
	}
	SH_GAME_API void RigidBody::SetCollider(Collider* colliderComponent)
	{
		if (impl->rigidbody == nullptr)
			return;

		if (collision != nullptr)
		{
			collision->onDestroy.UnRegister(colliderDestroyListener);
			auto& handles = collision->handles;
			auto it = std::find(handles.begin(), handles.end(), Collider::Handle{ this, impl->collider });
			if (it != handles.end())
				handles.erase(it);
		}

		collision = colliderComponent;

		if (collision != nullptr)
			collision->onDestroy.Register(colliderDestroyListener);

		if (impl->collider != nullptr)
		{
			impl->rigidbody->removeCollider(impl->collider);
			impl->collider = nullptr;
		}

		if (!core::IsValid(collision))
			return;

		const auto ComputeLocalToBodyFn = 
			[&](const GameObject& colliderGO) -> reactphysics3d::Transform
			{
				const glm::vec3 rbWorldPos = gameObject.transform->GetWorldPosition();
				glm::quat rbWorldQuat = gameObject.transform->GetWorldQuat();
				rbWorldQuat = glm::normalize(rbWorldQuat);

				const glm::vec3 colWorldPos = colliderGO.transform->GetWorldPosition();
				glm::quat colWorldQuat = colliderGO.transform->GetWorldQuat();
				colWorldQuat = glm::normalize(colWorldQuat);

				const glm::quat invRb = glm::inverse(rbWorldQuat);
				glm::quat localQuat = invRb * colWorldQuat;
				localQuat = glm::normalize(localQuat);

				const glm::vec3 localPos = invRb * (colWorldPos - rbWorldPos);

				return reactphysics3d::Transform{
					reactphysics3d::Vector3{ localPos.x, localPos.y, localPos.z },
					reactphysics3d::Quaternion{ localQuat.x, localQuat.y, localQuat.z, localQuat.w }
				};
			};
		reactphysics3d::Transform localToBody = reactphysics3d::Transform::identity();

		auto shape = reinterpret_cast<reactphysics3d::CollisionShape*>(collision->GetNative());

		// 리지드 바디랑 콜라이더랑 같은 오브젝트에 있으면 transform은 identity
		if (&colliderComponent->gameObject != &gameObject)
			localToBody = ComputeLocalToBodyFn(collision->gameObject);

		impl->collider = impl->rigidbody->addCollider(shape, localToBody);
		impl->collider->getMaterial().setBounciness(bouncy);
		impl->collider->setCollisionCategoryBits(static_cast<uint16_t>(collision->GetCollisionTag()));
		impl->collider->setCollideWithMaskBits(collision->GetAllowCollisions());
		impl->collider->setIsTrigger(collision->IsTrigger());

		collision->handles.push_back({ this, impl->collider });
	}

	SH_GAME_API auto RigidBody::GetCollider() const -> Collider*
	{
		return collision;
	}

	SH_GAME_API void RigidBody::SetMass(float mass)
	{
		this->mass = mass;
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
		linearDamping = damping;
		impl->rigidbody->setLinearDamping(damping);
	}

	SH_GAME_API void RigidBody::SetAngularDamping(float damping)
	{
		angularDamping = damping;
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

	SH_GAME_API void RigidBody::SetAngularLock(const game::Vec3& dir)
	{
		angularLock = dir;
		angularLock.x = std::clamp(std::roundf(angularLock.x), 0.f, 1.f);
		angularLock.y = std::clamp(std::roundf(angularLock.y), 0.f, 1.f);
		angularLock.z = std::clamp(std::roundf(angularLock.z), 0.f, 1.f);

		// reactPhysics에선 0이 허용, 1이 잠금이기 때문에 반전 시켜야함.
		impl->rigidbody->setAngularLockAxisFactor({ 1.0f - angularLock.x , 1.0f - angularLock.y, 1.0f - angularLock.z });
	}
	SH_GAME_API auto RigidBody::GetAngularLock() const -> const game::Vec3&
	{
		return angularLock;
	}
	SH_GAME_API void RigidBody::SetAxisLock(const game::Vec3& dir)
	{
		axisLock = dir;
		axisLock.x = std::clamp(std::roundf(axisLock.x), 0.f, 1.f);
		axisLock.y = std::clamp(std::roundf(axisLock.y), 0.f, 1.f);
		axisLock.z = std::clamp(std::roundf(axisLock.z), 0.f, 1.f);

		// reactPhysics에선 0이 허용, 1이 잠금이기 때문에 반전 시켜야함.
		impl->rigidbody->setLinearLockAxisFactor({ 1.0f - axisLock.x, 1.0f - axisLock.y, 1.0f - axisLock.z });
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

	SH_GAME_API void RigidBody::SetInterpolation(bool bUse)
	{
		this->bInterpolation = bUse;
	}

	SH_GAME_API auto RigidBody::GetInterpolation() const -> bool
	{
		return bInterpolation;
	}

	SH_GAME_API auto RigidBody::GetNativeHandle() const -> RigidBodyHandle
	{
		return impl->rigidbody;
	}

	SH_GAME_API void RigidBody::ResetPhysicsTransform()
	{
		gameObject.transform->UpdateMatrix();

		const Vec3& rbWorldPos = gameObject.transform->GetWorldPosition();
		const glm::quat& rbWorldQuat = gameObject.transform->GetWorldQuat();

		if (core::IsValid(collision) && impl->collider != nullptr)
		{
			if (&collision->gameObject != &gameObject)
			{
				const glm::vec3 colWPos = collision->gameObject.transform->GetWorldPosition();
				auto colWQuat = collision->gameObject.transform->GetWorldQuat();
				colWQuat = glm::normalize(colWQuat);

				const glm::quat invRb = glm::inverse(rbWorldQuat);
				glm::quat localQuat = glm::normalize(invRb * colWQuat);
				const glm::vec3 localPos = invRb * (colWPos - glm::vec3(rbWorldPos.x, rbWorldPos.y, rbWorldPos.z));

				impl->collider->setLocalToBodyTransform(
					reactphysics3d::Transform
					{
						reactphysics3d::Vector3{ localPos.x, localPos.y, localPos.z },
						reactphysics3d::Quaternion{ localQuat.x, localQuat.y, localQuat.z, localQuat.w }
					}
				);
			}
			else
			{
				impl->collider->setLocalToBodyTransform(reactphysics3d::Transform::identity());
			}
		}

		// rb 월드 트랜스폼 리셋
		impl->rigidbody->setTransform(
			reactphysics3d::Transform{
				reactphysics3d::Vector3{ rbWorldPos.x, rbWorldPos.y, rbWorldPos.z },
				reactphysics3d::Quaternion{ rbWorldQuat.x, rbWorldQuat.y, rbWorldQuat.z, rbWorldQuat.w }
			}
		);

		ResetInterpolationState();
	}

	SH_GAME_API void RigidBody::ResetInterpolationState()
	{
		const Vec3& objPos = gameObject.transform->GetWorldPosition();
		const auto& objQuat = gameObject.transform->GetWorldQuat();

		prevPos = objPos;
		prevRot = objQuat;
		currPos = objPos;
		currRot = objQuat;
	}

	SH_GAME_API auto RigidBody::GetPhysicsPosition() const -> game::Vec3
	{
		const auto& p = impl->rigidbody->getTransform().getPosition();
		return { p.x, p.y, p.z };
	}

	SH_GAME_API void RigidBody::OnPropertyChanged(const core::reflection::Property& prop)
	{
		Super::OnPropertyChanged(prop);
		if (prop.GetName() == core::Util::ConstexprHash("collision"))
		{
			SetCollider(collision);
		}
		else if (prop.GetName() == core::Util::ConstexprHash("bGravity"))
		{
			SetUsingGravity(bGravity);
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
		else if (prop.GetName() == core::Util::ConstexprHash("angularLock"))
		{
			SetAngularLock(angularLock);
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
	SH_GAME_API auto RigidBody::GetRigidBodyFromHandle(RigidBodyHandle handle) -> RigidBody*
	{
		auto it = nativeMap.find(handle);
		if (it == nativeMap.end())
			return nullptr;
		return it->second;
	}
	void RigidBody::Interpolate()
	{
		// 바뀌지 않았으니 보간x
		if (prevPos == currPos && prevRot == currRot)
			return;

		if (!bKinematic && bInterpolation)
		{
			float alpha = std::clamp(gameObject.world.fixedDeltaTime / gameObject.world.FIXED_TIME, 0.f, 1.f);
			glm::vec3 interpPos = glm::mix(prevPos, currPos, alpha);
			glm::quat interpRot = glm::slerp(prevRot, currRot, alpha);
			interpRot = glm::normalize(interpRot);

			gameObject.transform->SetWorldPosition(interpPos);
			gameObject.transform->SetWorldRotation(interpRot);
		}
		else
		{
			gameObject.transform->SetWorldPosition(currPos);
			gameObject.transform->SetWorldRotation(currRot);
		}
		gameObject.transform->UpdateMatrix();
	}
}//namespace