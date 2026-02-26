#include "Component/Phys/RigidBody.h"
#include "Component/Phys/Collider.h"
#include "World.h"

#include "Core/Logger.h"

#include "Physics/PhysWorld.h"

#include "reactphysics3d/reactphysics3d.h"

#include <cstdint>
namespace sh::game
{
	std::unordered_map<RigidBody::RigidBodyHandle, RigidBody*> RigidBody::nativeMap{};

	struct RigidBody::Impl
	{
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

		lastState.torque = { 0.f, 0.f, 0.f };
		lastState.vel = { 0.f, 0.f, 0.f };
		lastState.angularVel = { 0.f, 0.f, 0.f };
	}
	SH_GAME_API RigidBody::~RigidBody()
	{
		SH_INFO("~RigidBody");
	}
	SH_GAME_API void RigidBody::Awake()
	{
		if (!gameObject.IsActive() || !IsActive())
			impl->rigidbody->setIsActive(false);
		//impl->rigidbody->setLinearVelocity({ 0.f, 0.f, 0.f });
	}
	SH_GAME_API void RigidBody::Start()
	{
		ResetPhysicsTransform();
		auto colliders = gameObject.GetComponentsInChildren<Collider>(true);
		for (Collider* collider : colliders)
		{
			if (!core::IsValid(collider->GetRigidbody()))
				collider->Setup();
		}
	}
	SH_GAME_API void RigidBody::OnEnable()
	{
		impl->rigidbody->setIsActive(true);
		impl->rigidbody->setIsSleeping(false);
		impl->rigidbody->applyWorldTorque({ lastState.torque.x, lastState.torque.y, lastState.torque.z });
		impl->rigidbody->setLinearVelocity({ lastState.vel.x, lastState.vel.y, lastState.vel.z });
		impl->rigidbody->setAngularVelocity({ lastState.angularVel.x, lastState.angularVel.y, lastState.angularVel.z });
	}
	SH_GAME_API void RigidBody::OnDisable()
	{
		impl->rigidbody->setIsActive(false);
		const auto& torque = impl->rigidbody->getTorque();
		const auto vel = impl->rigidbody->getLinearVelocity();
		const auto avel = impl->rigidbody->getAngularVelocity();
		lastState.torque = { torque.x, torque.y, torque.z };
		lastState.vel = { vel.x, vel.y, vel.z };
		lastState.angularVel = { avel.x, avel.y, avel.z };
	}
	SH_GAME_API void RigidBody::OnDestroy()
	{
		nativeMap.erase(impl->rigidbody);

		auto physWorld = reinterpret_cast<reactphysics3d::PhysicsWorld*>(gameObject.world.GetPhysWorld()->GetNative());
		physWorld->destroyRigidBody(impl->rigidbody);
		impl->rigidbody = nullptr;

		Super::OnDestroy();
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
	SH_GAME_API void RigidBody::OnPropertyChanged(const core::reflection::Property& prop)
	{
		Super::OnPropertyChanged(prop);
		if (prop.GetName() == core::Util::ConstexprHash("bGravity"))
			SetUsingGravity(bGravity);
		else if (prop.GetName() == core::Util::ConstexprHash("bKinematic"))
			SetKinematic(bKinematic);
		else if (prop.GetName() == core::Util::ConstexprHash("mass"))
			SetMass(mass);
		else if (prop.GetName() == core::Util::ConstexprHash("linearDamping"))
			SetLinearDamping(linearDamping);
		else if (prop.GetName() == core::Util::ConstexprHash("angularDamping"))
			SetAngularDamping(angularDamping);
		else if (prop.GetName() == core::Util::ConstexprHash("angularLock"))
			SetAngularLock(angularLock);
		else if (prop.GetName() == core::Util::ConstexprHash("axisLock"))
			SetAxisLock(axisLock);
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
		const auto& f = impl->rigidbody->getForce();
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

		const Vec3& worldPos = gameObject.transform->GetWorldPosition();
		const glm::quat& worldQuat = gameObject.transform->GetWorldQuat();

		// rb 월드 트랜스폼 리셋
		impl->rigidbody->setTransform(
			reactphysics3d::Transform{
				reactphysics3d::Vector3{ worldPos.x, worldPos.y, worldPos.z },
				reactphysics3d::Quaternion{ worldQuat.x, worldQuat.y, worldQuat.z, worldQuat.w }
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

	SH_GAME_API auto RigidBody::CheckOverlap(const RigidBody& other) const -> bool
	{
		if (this == &other)
			return true;

		auto world = reinterpret_cast<reactphysics3d::PhysicsWorld*>(gameObject.world.GetPhysWorld()->GetNative());
		return world->testOverlap(impl->rigidbody, other.impl->rigidbody);
	}

	SH_GAME_API auto RigidBody::GetRigidBodyUsingHandle(RigidBodyHandle handle) -> RigidBody*
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
			float alpha = std::clamp(gameObject.world.GetFixedAccumulator() / gameObject.world.FIXED_TIME, 0.f, 1.f);
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