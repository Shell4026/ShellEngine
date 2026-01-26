#pragma once
#include "Game/Export.h"
#include "Collider.h"

#include "Game/Vector.h"

#include "Core/SContainer.hpp"

#include <memory>
namespace sh::game
{
	class RigidBody;
	class DebugRenderer;
	class ShpereCollider : public Collider
	{
		COMPONENT(ShpereCollider, "physics")
		friend RigidBody;
	public:
		SH_GAME_API ShpereCollider(GameObject& owner);
		SH_GAME_API ~ShpereCollider();

		SH_GAME_API void OnDestroy() override;

		SH_GAME_API auto GetNative() const -> void* override;

		SH_GAME_API void SetRadius(float r);
		SH_GAME_API auto GetRadius() const -> float;

		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& prop) override;

		SH_GAME_API void Update() override;

		SH_GAME_API void DisplayArea(bool bDisplay = true);
	private:
		struct Impl;
		std::unique_ptr<Impl> impl;

		DebugRenderer* debugRenderer = nullptr;
		PROPERTY(bDisplayArea)
		bool bDisplayArea = false;

		PROPERTY(radius)
		float radius;
	};
}//namespace
