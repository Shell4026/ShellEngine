#pragma once
#include "Export.h"
#include "Component.h"
#include "Collider.h"
#include "../Vector.h"

#include "Core/Observer.hpp"
#include "Core/EventSubscriber.h"

#include "Physics/PhysWorld.h"

#include <glm/gtc/quaternion.hpp>

#include <unordered_map>
namespace sh::game
{
	class RigidBody : public Component
	{
		COMPONENT(RigidBody)
	public:
		using RigidBodyHandle = void*;
	public:
		SH_GAME_API RigidBody(GameObject& owner);
		SH_GAME_API ~RigidBody();

		SH_GAME_API void Start() override;
		SH_GAME_API void OnDestroy() override;
		SH_GAME_API void BeginUpdate() override;
		SH_GAME_API void FixedUpdate() override;
		SH_GAME_API void Update() override;
		SH_GAME_API void LateUpdate() override;

		/// @brief 물리 법칙에 따라 움직이지만 질량이 무한대인 상태로 설정할지
		/// @param set true 또는 false
		SH_GAME_API void SetKinematic(bool set);
		SH_GAME_API void SetUsingGravity(bool use);

		SH_GAME_API void SetCollider(Collider* colliderComponent);
		SH_GAME_API auto GetCollider() const -> Collider*;

		SH_GAME_API void SetMass(float mass);
		SH_GAME_API void SetLinearVelocity(const game::Vec3& v);
		SH_GAME_API void SetAngularVelocity(const game::Vec3& v);
		SH_GAME_API void SetLinearDamping(float damping);
		SH_GAME_API void SetAngularDamping(float damping);
		SH_GAME_API void AddWorldTorque(const game::Vec3& torque);
		SH_GAME_API void AddWorldForce(const game::Vec3& force);
		SH_GAME_API void AddTorque(const game::Vec3& torque);
		SH_GAME_API void AddForce(const game::Vec3& force);
		/// @brief 특정 축의 움직임을 제한한다.
		/// @param dir 1이면 해당 축의 움직임을 제한, 0이면 허용
		SH_GAME_API void SetAxisLock(const game::Vec3& dir);
		SH_GAME_API auto GetAxisLock() const -> const game::Vec3&;
		SH_GAME_API void SetBouncy(float bouncy);
		SH_GAME_API auto GetBouncy() const -> float;

		SH_GAME_API bool IsKinematic() const;
		SH_GAME_API bool IsGravityUse() const;

		SH_GAME_API auto GetMass() const -> float;
		SH_GAME_API auto GetLinearDamping() const -> float;
		SH_GAME_API auto GetAngularDamping() const -> float;
		SH_GAME_API auto GetLinearVelocity() const -> game::Vec3;
		SH_GAME_API auto GetAngularVelocity() const -> game::Vec3;
		SH_GAME_API auto GetForce() const -> game::Vec3;

		SH_GAME_API void SetSleep();

		SH_GAME_API void SetInterpolation(bool bUse);
		SH_GAME_API auto GetInterpolation() const -> bool;

		SH_GAME_API auto GetNativeHandle() const -> RigidBodyHandle;

		/// @brief 물리 객체의 transform을 현재 오브젝트의 transform으로 초기화 하는 코드
		SH_GAME_API void ResetPhysicsTransform();
		SH_GAME_API void ResetInterpolationState();

		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& prop) override;

		SH_GAME_API static auto GetRigidBodyFromHandle(RigidBodyHandle handle) -> RigidBody*;
	private:
		void Interpolate();
	private:
		struct Impl;

		PROPERTY(collision)
		Collider* collision = nullptr;

		std::unique_ptr<Impl> impl;

		PROPERTY(axisLock)
		game::Vec3 axisLock{ 0.f, 0.f, 0.f };
		PROPERTY(mass)
		float mass = 1.0f;
		PROPERTY(bouncy)
		float bouncy = 0.2f;
		PROPERTY(linearDamping)
		float linearDamping = 0.f;
		PROPERTY(angularDamping)
		float angularDamping = 0.1f;
		PROPERTY(bKinematic)
		bool bKinematic = false;
		PROPERTY(bGravity)
		bool bGravity = false;
		PROPERTY(bInterpolation)
		bool bInterpolation = true;

		glm::vec3 prevPos;
		glm::vec3 currPos;
		glm::quat prevRot;
		glm::quat currRot;

		core::Observer<false, const SObject*>::Listener colliderDestroyListener;
		core::EventSubscriber<phys::PhysWorld::PhysicsEvent> physEventSubscriber;

		static std::unordered_map<RigidBodyHandle, RigidBody*> nativeMap;
	};
}//namespace