#pragma once
#include "../Export.h"
#include "Component.h"

#include "Core/SContainer.hpp"

#include "Physics/CollisionTag.hpp"

#include <vector>
namespace sh::game
{
	class Collider : public Component
	{
		SCLASS(Collider)
		friend class RigidBody;
	public:
		SH_GAME_API Collider(GameObject& owner);
		SH_GAME_API virtual ~Collider() = default;

		SH_GAME_API virtual auto GetNative() const -> void* = 0;

		SH_GAME_API void SetTrigger(bool bTrigger);
		SH_GAME_API auto IsTrigger() const -> bool;
		SH_GAME_API void SetCollisionTag(phys::Tag tag);
		SH_GAME_API auto GetCollisionTag() const -> phys::Tag;
		SH_GAME_API void SetAllowCollisions(phys::Tagbit tags);
		SH_GAME_API auto GetAllowCollisions() const -> phys::Tagbit;
		/// @brief 연결된 유효한 리지드 바디들을 반환한다.
		SH_GAME_API auto GetRigidbodies() const -> std::vector<RigidBody*>;

		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& prop) override;
	private:
		phys::Tag tag = phys::Tag::Tag1;
		phys::Tagbit allowed = 0xffff;

		struct Handle
		{
			SH_GAME_API ~Handle();
			auto operator==(const Handle& other) const -> bool
			{
				return rb == other.rb && nativeCollider == other.nativeCollider;
			}

			core::SObjWeakPtr<RigidBody, void> rb;
			void* nativeCollider = nullptr;
		};
		std::vector<Handle> handles;

		PROPERTY(bTrigger)
		bool bTrigger = false;
	};
}//namespace