#pragma once
#include "Export.h"
#include "Component.h"
#include "Collider.h"
#include "../Vector.h"

namespace sh::game
{
	class RigidBody : public Component
	{
		COMPONENT(RigidBody)
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
		SH_GAME_API void SetMass(float mass);
		SH_GAME_API void SetLinearVelocity(const game::Vec3& v);
		SH_GAME_API void SetAngularVelocity(const game::Vec3& v);
		SH_GAME_API void SetLinearDamping(float damping);
		SH_GAME_API void SetAngularDamping(float damping);
		SH_GAME_API void AddWorldTorque(const game::Vec3& torque);
		SH_GAME_API void AddWorldForce(const game::Vec3& force);
		SH_GAME_API void AddTorque(const game::Vec3& torque);
		SH_GAME_API void AddForce(const game::Vec3& force);

		SH_GAME_API bool IsKinematic() const;
		SH_GAME_API bool IsGravityUse() const;

		SH_GAME_API auto GetMass() const -> float;
		SH_GAME_API auto GetLinearDamping() const -> float;
		SH_GAME_API auto GetAngularDamping() const -> float;
		SH_GAME_API auto GetLinearVelocity() const -> game::Vec3;
		SH_GAME_API auto GetAngularVelocity() const -> game::Vec3;
		SH_GAME_API auto GetForce() const -> game::Vec3;

		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& prop) override;
	private:
		struct Impl;

		PROPERTY(collision)
		Collider* collision = nullptr;
		Collider* collisionLast = nullptr;

		std::unique_ptr<Impl> impl;

		PROPERTY(mass)
		float mass = 1.0f;
		PROPERTY(linearDamping)
		float linearDamping = 0.f;
		PROPERTY(angularDamping)
		float angularDamping = 0.1f;
		PROPERTY(bKinematic)
		bool bKinematic = false;
		PROPERTY(bGravity)
		bool bGravity = false;
	};
}//namespace