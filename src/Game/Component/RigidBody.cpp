#include "PCH.h"
#include "Component/RigidBody.h"

#include "GameObject.h"
#include "PhysWorld.h"

#include "Core/Logger.h"

namespace sh::game
{
	SH_GAME_API RigidBody::RigidBody(GameObject& owner) :
		Component(owner)
	{
		auto& ctx = gameObject.world.GetPhysWorld()->GetContext();
		auto world = gameObject.world.GetPhysWorld()->GetWorld();

		reactphysics3d::Vector3 physPos{ gameObject.transform->position.x, gameObject.transform->position.y, gameObject.transform->position.z };
		auto& quat = gameObject.transform->GetQuat();
		reactphysics3d::Quaternion physQuat{ quat.x, quat.y, quat.z, quat.w };
		reactphysics3d::Transform transform{ physPos, physQuat };

		rigidbody = world->createRigidBody(transform);
		rigidbody->setType(reactphysics3d::BodyType::DYNAMIC);
		rigidbody->enableGravity(bGravity);
	}
	SH_GAME_API RigidBody::~RigidBody()
	{
		SH_INFO("~RigidBody");
	}
	SH_GAME_API void RigidBody::OnDestroy()
	{
		SH_INFO("Destory!!!");
		if (collider)
			rigidbody->removeCollider(collider);

		auto world = gameObject.world.GetPhysWorld()->GetWorld();
		gameObject.world.GetPhysWorld()->DestroyRigidBody(rigidbody);
		rigidbody = nullptr;
	}
	SH_GAME_API void RigidBody::BeginUpdate()
	{
		if(collider && !core::IsValid(collision))
			SetCollider(nullptr);
		auto& objQuat = gameObject.transform->GetQuat();
		rigidbody->setTransform(reactphysics3d::Transform{ gameObject.transform->position, reactphysics3d::Quaternion{objQuat.x, objQuat.y, objQuat.z, objQuat.w} });
	}
	SH_GAME_API void RigidBody::FixedUpdate()
	{
		auto& pos = rigidbody->getTransform().getPosition();
		auto& quat = rigidbody->getTransform().getOrientation();
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
			rigidbody->setType(reactphysics3d::BodyType::KINEMATIC);
		else
			rigidbody->setType(reactphysics3d::BodyType::DYNAMIC);
	}
	SH_GAME_API void RigidBody::SetGravity(bool use)
	{
		bGravity = use;
		rigidbody->enableGravity(bGravity);
	}
	SH_GAME_API void RigidBody::SetCollider(BoxCollider* colliderComponent)
	{
		collision = colliderComponent;
		if (collision == collisionLast)
			return;
		if (rigidbody == nullptr)
			return;

		if (core::IsValid(collisionLast))
			collisionLast->rigidbodies.erase(this);

		if (collider)
		{
			rigidbody->removeCollider(collider);
			collider = nullptr;
		}

		if (core::IsValid(collision))
		{
			collision->rigidbodies.insert(this);
			collider = rigidbody->addCollider(**collision, reactphysics3d::Transform::identity());
		}
		collisionLast = collision;
	}

	SH_GAME_API bool RigidBody::IsKinematic() const
	{
		return bKinematic;
	}
	SH_GAME_API bool RigidBody::IsGravityUse() const
	{
		return bGravity;
	}

#if SH_EDITOR
	SH_GAME_API void RigidBody::OnPropertyChanged(const core::reflection::Property& prop)
	{
		if (std::strcmp(prop.GetName(), "collision") == 0)
		{
			SetCollider(collision);
		}
		else if (std::strcmp(prop.GetName(), "bGravity") == 0)
		{
			SetGravity(bGravity);
		}
		else if (std::strcmp(prop.GetName(), "bKinematic") == 0)
		{
			SetKinematic(bKinematic);
		}
	}
#endif
}//namespace