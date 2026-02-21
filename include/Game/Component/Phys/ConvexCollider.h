#pragma once
#include "Game/Export.h"
#include "Game/Component/Phys/Collider.h"

#include "Game/Vector.h"

#include "Core/SContainer.hpp"
namespace sh::render
{
	class Mesh;
}
namespace sh::game
{
	class RigidBody;
	class DebugRenderer;
	class ConvexCollider : public Collider
	{
		COMPONENT(ConvexCollider, "physics")
	public:
		SH_GAME_API ConvexCollider(GameObject& owner);
		SH_GAME_API ~ConvexCollider();

		SH_GAME_API void Start() override;
		SH_GAME_API void OnDestroy() override;

		SH_GAME_API void OnPropertyChanged(const core::reflection::Property& prop) override;

		SH_GAME_API void Update() override;

		//SH_GAME_API void DisplayArea(bool bDisplay = true);
	protected:
		SH_GAME_API void DestroyShape() override;
		SH_GAME_API auto GetShape() const -> void* override;
	private:
		void CreateShape();
	private:
		struct Impl;
		std::unique_ptr<Impl> impl;

		PROPERTY(mesh)
		const render::Mesh* mesh = nullptr;
		const render::Mesh* lastMesh = nullptr;

		//DebugRenderer* debugRenderer = nullptr;
		//PROPERTY(bDisplayArea)
		//bool bDisplayArea = false;
	};
}//namespace
