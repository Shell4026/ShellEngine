#pragma once
#include "Export.h"
#include "Component.h"

#include "Core/SContainer.hpp"
namespace sh::game
{
	class RigidBody;
	class Collider : public Component
	{
		SCLASS(Collider)
		friend RigidBody;
	protected:
		core::SSet<RigidBody*> rigidbodies;
	public:
		SH_GAME_API Collider(GameObject& owner);
		SH_GAME_API virtual ~Collider() = default;

		SH_GAME_API virtual auto GetNative() const -> void* = 0;
	};
}//namespace