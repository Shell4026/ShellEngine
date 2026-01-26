#pragma once
#include "Game/Export.h"
#include "Game/Vector.h"
#include "Game/Component/Phys/Collider.h"

#include "Core/SContainer.hpp"

#include <memory>
namespace sh::game
{
	class RigidBody;
	class DebugRenderer;
	class CapsuleCollider : public Collider
	{
		COMPONENT(CapsuleCollider, "physics")
		friend RigidBody;
	public:
		SH_GAME_API CapsuleCollider(GameObject& owner);
		SH_GAME_API ~CapsuleCollider();

		SH_GAME_API void OnDestroy() override;
		SH_GAME_API void Awake() override;

		SH_GAME_API auto GetNative() const -> void* override;

		SH_GAME_API void SetRadius(float r);
		SH_GAME_API auto GetRadius() const -> float;
		SH_GAME_API void SetHeight(float h);
		SH_GAME_API auto GetHeight() const -> float;

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
		float radius = 0.5f;
		PROPERTY(height)
		float height = 1.f;
	};
}//namespace
