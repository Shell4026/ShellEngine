#include "Component/Phys/Collider.h"
#include "Component/Phys/RigidBody.h"
#include "Game/World.h"
#include "Game/Vector.h"
#include "reactphysics3d/reactphysics3d.h"

#include <algorithm>
namespace sh::game
{
	std::unordered_map<Collider::ColliderHandle, Collider*> Collider::nativeColliderMap;

	Collider::Collider(GameObject& owner) :
		Component(owner)
	{
		onRigidbodyDestroyListener.SetCallback(
			[this](const core::SObject* obj)
			{
				rigidBody = nullptr;
				nativeColliderMap.erase(nativeCollider);
				nativeCollider = nullptr;

				if (core::IsValid(this))
					Setup();
			}
		);
		canPlayInEditor = true;
	}
	SH_GAME_API void Collider::Awake()
	{
	}
	SH_GAME_API void Collider::OnDestroy()
	{
		if (rigidBody != nullptr)
		{
			rigidBody->onDestroy.UnRegister(onRigidbodyDestroyListener);
			RemoveCollider();
			rigidBody = nullptr;
		}
		DestroyShape();
		Super::OnDestroy();
	}
	SH_GAME_API void Collider::SetTrigger(bool bTrigger)
	{
		this->bTrigger = bTrigger;
		if (nativeCollider != nullptr)
		{
			auto collider = reinterpret_cast<reactphysics3d::Collider*>(nativeCollider);
			collider->setIsTrigger(this->bTrigger);
		}
	}
	SH_GAME_API void Collider::SetCollisionTag(phys::Tag tag)
	{
		this->tag = tag;
		if (nativeCollider != nullptr)
		{
			auto collider = reinterpret_cast<reactphysics3d::Collider*>(nativeCollider);
			collider->setCollisionCategoryBits(this->tag);
		}
	}
	SH_GAME_API void Collider::SetAllowCollisions(phys::Tagbit tags)
	{
		allowed = tags;
		if (nativeCollider != nullptr)
		{
			auto collider = reinterpret_cast<reactphysics3d::Collider*>(nativeCollider);
			collider->setCollideWithMaskBits(allowed);
		}
	}
	SH_GAME_API void Collider::SetBouncy(float bouncy)
	{
		this->bouncy = std::clamp(bouncy, 0.f, 1.f);
		if (nativeCollider != nullptr)
		{
			reinterpret_cast<reactphysics3d::Collider*>(nativeCollider)->getMaterial().setBounciness(this->bouncy);
		}
	}
	SH_GAME_API void Collider::SetFriction(float friction)
	{
		this->friction = std::clamp(bouncy, 0.f, 1.f);
		if (nativeCollider != nullptr)
		{
			reinterpret_cast<reactphysics3d::Collider*>(nativeCollider)->getMaterial().setFrictionCoefficient(this->friction);
		}
	}
	SH_GAME_API void Collider::OnPropertyChanged(const core::reflection::Property& prop)
	{
		Super::OnPropertyChanged(prop);
		if (prop.GetName() == core::Util::ConstexprHash("bouncy"))
			SetBouncy(bouncy);
		else if (prop.GetName() == core::Util::ConstexprHash("friction"))
			SetFriction(friction);
	}

	SH_GAME_API auto Collider::GetColliderUsingHandle(ColliderHandle handle) -> Collider*
	{
		auto it = nativeColliderMap.find(handle);
		if (it == nativeColliderMap.end())
			return nullptr;
		return it->second;
	}

	void Collider::SetupRigidbody()
	{
		if (rigidBody != nullptr)
		{
			rigidBody->onDestroy.UnRegister(onRigidbodyDestroyListener);
			RemoveCollider();
			rigidBody = nullptr;
		}

		RigidBody* rb = nullptr;
		Transform* cur = gameObject.transform;
		while (cur != nullptr)
		{
			rb = cur->gameObject.GetComponent<RigidBody>();
			if (core::IsValid(rb))
				break;
			cur = cur->GetParent();
		}
		if (!core::IsValid(rb))
			return;

		rigidBody = rb;
		rigidBody->onDestroy.Register(onRigidbodyDestroyListener);
	}
	void Collider::SetupCollider()
	{
		if (!core::IsValid(rigidBody))
			return;
		auto rbHandle = reinterpret_cast<reactphysics3d::RigidBody*>(rigidBody->GetNativeHandle());
		auto shape = reinterpret_cast<reactphysics3d::CollisionShape*>(GetShape());
		reactphysics3d::Transform transform = reactphysics3d::Transform::identity();
		if (&rigidBody->gameObject != &gameObject)
		{
			if (rigidBody->gameObject.transform->IsAncestorOf(*gameObject.transform))
			{
				rigidBody->gameObject.transform->UpdateMatrix();
				gameObject.transform->UpdateMatrix();

				const glm::vec3 rbWorldPos = rigidBody->gameObject.transform->GetWorldPosition();
				const glm::vec3 worldPos = gameObject.transform->GetWorldPosition();
				const glm::quat rbWorldQuat = rigidBody->gameObject.transform->GetWorldQuat();
				const glm::quat worldQuat = gameObject.transform->GetWorldQuat();

				const glm::quat invRbWorldQuat = glm::inverse(rbWorldQuat);

				const glm::vec3 localPos = invRbWorldQuat * (worldPos - rbWorldPos);
				transform.setPosition({ localPos.x, localPos.y, localPos.z });
				const glm::quat localQuat = invRbWorldQuat * worldQuat;
				transform.setOrientation({ localQuat.x, localQuat.y, localQuat.z, localQuat.w });
			}
			else
			{
				SH_ERROR_FORMAT("{}: Rigidbody must be ancestor of collider!", gameObject.GetName().ToString());
				return;
			}
		}
		nativeCollider = rbHandle->addCollider(shape, transform);
		nativeColliderMap.insert({ nativeCollider, this });
		reinterpret_cast<reactphysics3d::Collider*>(nativeCollider)->getMaterial().setBounciness(bouncy);
	}
	void Collider::RemoveCollider()
	{
		if (rigidBody != nullptr && nativeCollider != nullptr)
		{
			auto rbHandle = reinterpret_cast<reactphysics3d::RigidBody*>(rigidBody->GetNativeHandle());
			rbHandle->removeCollider(reinterpret_cast<reactphysics3d::Collider*>(nativeCollider));
			nativeColliderMap.erase(nativeCollider);
			nativeCollider = nullptr;
		}
	}
	void Collider::Setup()
	{
		SetupRigidbody();
		SetupCollider();

		if (nativeCollider == nullptr)
			return;

		auto collider = reinterpret_cast<reactphysics3d::Collider*>(nativeCollider);
		collider->setIsTrigger(bTrigger);
		collider->setCollisionCategoryBits(tag);
		collider->setCollideWithMaskBits(allowed);
	}
}//namespace