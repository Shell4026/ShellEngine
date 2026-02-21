#pragma once
#include "Game/Export.h"
#include "Game/Component/Component.h"

#include "Core/Observer.hpp"

#include "Physics/CollisionTag.hpp"

#include <vector>
#include <unordered_map>
namespace sh::game
{
	class RigidBody;
	class Collider : public Component
	{
		SCLASS(Collider)
		friend RigidBody;
	public:
		using ColliderHandle = void*;
	public:
		SH_GAME_API Collider(GameObject& owner);
		SH_GAME_API virtual ~Collider() = default;

		SH_GAME_API void Awake() override;
		SH_GAME_API void OnDestroy() override;

		SH_GAME_API void SetTrigger(bool bTrigger);
		SH_GAME_API void SetCollisionTag(phys::Tag tag);
		SH_GAME_API void SetAllowCollisions(phys::Tagbit tags);
		SH_GAME_API void SetBouncy(float bouncy);
		SH_GAME_API void SetFriction(float friction);
		
		SH_GAME_API auto GetCollisionTag() const -> phys::Tag { return tag; }
		SH_GAME_API auto GetAllowCollisions() const -> phys::Tagbit { return allowed; }
		SH_GAME_API auto GetRigidbody() const -> RigidBody* { return rigidBody; }
		SH_GAME_API auto GetBouncy() const -> float { return bouncy; }
		SH_GAME_API auto GetFriction() const -> float { return friction; }
		SH_GAME_API auto IsTrigger() const -> bool { return bTrigger; }
		SH_GAME_API auto GetNativeHandle() -> ColliderHandle { return nativeCollider; }

		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& prop) override;

		SH_GAME_API static auto GetColliderUsingHandle(ColliderHandle handle) -> Collider*;
	protected:
		SH_GAME_API virtual void DestroyShape() = 0;
		SH_GAME_API virtual auto GetShape() const -> void* = 0;
	private:
		void SetupRigidbody();
		void SetupCollider();
		void RemoveCollider();
		void Setup();
	private:
		PROPERTY(bouncy)
		float bouncy = 0.2f;
		PROPERTY(friction)
		float friction = 0.2f;
		PROPERTY(rigidBody, core::PropertyOption::sobjPtr, core::PropertyOption::invisible)
		RigidBody* rigidBody = nullptr;
		void* nativeCollider = nullptr;

		phys::Tag tag = phys::Tag::Tag1;
		phys::Tagbit allowed = 0xffff;

		core::Observer<false, const SObject*>::Listener onRigidbodyDestroyListener;

		PROPERTY(bTrigger)
		bool bTrigger = false;

		static std::unordered_map<ColliderHandle, Collider*> nativeColliderMap;
	};
}//namespace