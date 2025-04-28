#pragma once

#include "Export.h"
#include "Component.h"

#include "Collider.h"

#include "reactphysics3d/reactphysics3d.h"
namespace sh::game
{
	class RigidBody : public Component
	{
		COMPONENT(RigidBody)
	private:
		PROPERTY(collision)
		Collider* collision = nullptr;
		Collider* collisionLast = nullptr;
		reactphysics3d::Collider* collider = nullptr;
		reactphysics3d::RigidBody* rigidbody = nullptr;

		PROPERTY(bKinematic)
		bool bKinematic = false;
		PROPERTY(bGravity)
		bool bGravity = false;
	public:
		SH_GAME_API RigidBody(GameObject& owner);
		SH_GAME_API ~RigidBody();

		SH_GAME_API void OnDestroy() override;
		SH_GAME_API void BeginUpdate() override;
		SH_GAME_API void FixedUpdate() override;
		SH_GAME_API void Update() override;
		SH_GAME_API void LateUpdate() override;

		/// @brief 물리 법칙에 따라 움직이지만 질량이 무한대인 상태로 설정할지
		/// @param set true 또는 false
		SH_GAME_API void SetKinematic(bool set);
		SH_GAME_API void SetGravity(bool use);
		SH_GAME_API void SetCollider(Collider* colliderComponent);

		SH_GAME_API bool IsKinematic() const;
		SH_GAME_API bool IsGravityUse() const;

		SH_GAME_API auto GetBody() const -> reactphysics3d::RigidBody*;
#if SH_EDITOR
		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& prop) override;
#endif
	};
}//namespace