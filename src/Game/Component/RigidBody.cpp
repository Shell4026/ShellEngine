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
	}
	SH_GAME_API RigidBody::~RigidBody()
	{
		SH_INFO("~RigidBody");
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
		if(impl->collider != nullptr && !core::IsValid(collision))
			SetCollider(nullptr);
		auto& objQuat = gameObject.transform->GetQuat();
		const Vec3& pos = gameObject.transform->position;
		impl->rigidbody->setTransform(reactphysics3d::Transform{ {pos.x, pos.y, pos.z}, reactphysics3d::Quaternion{objQuat.x, objQuat.y, objQuat.z, objQuat.w} });
	}
	SH_GAME_API void RigidBody::FixedUpdate()
	{
		auto& pos = impl->rigidbody->getTransform().getPosition();
		auto& quat = impl->rigidbody->getTransform().getOrientation();
		gameObject.transform->SetPosition(pos.x, pos.y, pos.z);
		gameObject.transform->SetRotation(glm::quat{ quat.w, quat.x, quat.y, quat.z });
	}
	SH_GAME_API void RigidBody::Update()
	{

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
		collision = colliderComponent;
		if (collision == collisionLast)
			return;
		if (impl->rigidbody == nullptr)
			return;

		if (core::IsValid(collisionLast))
			collisionLast->rigidbodies.erase(this);

		if (impl->collider)
		{
			impl->rigidbody->removeCollider(impl->collider);
			impl->collider = nullptr;
		}

		if (core::IsValid(collision))
		{
			collision->rigidbodies.insert(this);
			
			auto shape = reinterpret_cast<reactphysics3d::CollisionShape*>(collision->GetNative());
			impl->collider = impl->rigidbody->addCollider(shape, reactphysics3d::Transform::identity());
		}
		collisionLast = collision;
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
	}
}//namespace