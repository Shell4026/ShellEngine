#include "Component/SphereCollider.h"
#include "Component/RigidBody.h"
#include "Component/DebugRenderer.h"

#include "Game/GameObject.h"
#include "Game/World.h"

namespace sh::game
{
	SH_GAME_API ShpereCollider::ShpereCollider(GameObject& owner) :
		Collider(owner), radius(0.5)
	{
		shape = world.GetPhysWorld()->GetContext().createSphereShape(radius);
#if SH_EDITOR
		debugRenderer = gameObject.AddComponent<DebugRenderer>();
		debugRenderer->SetMesh(world.models.GetResource("SphereModel")->GetMeshes()[0]);
		debugRenderer->hideInspector = true;
#endif
	}
	SH_GAME_API ShpereCollider::~ShpereCollider()
	{
		SH_INFO("~ShpereCollider");
		world.GetPhysWorld()->GetContext().destroySphereShape(shape);
	}

	SH_GAME_API void ShpereCollider::Destroy()
	{
		Super::Destroy();
		debugRenderer->Destroy();
		debugRenderer = nullptr;
	}

	SH_GAME_API void ShpereCollider::OnDestroy()
	{
		SH_INFO("Destroy!");
		for (auto rb : rigidbodies)
		{
			rb->SetCollider(nullptr);
		}
	}

	SH_GAME_API auto ShpereCollider::GetCollisionShape() const -> reactphysics3d::CollisionShape*
	{
		return shape;
	}

	SH_GAME_API void ShpereCollider::SetRadius(float r)
	{
		radius = r;
		shape->setRadius(radius);
	}
	SH_GAME_API auto ShpereCollider::GetRadius() const -> float
	{
		return radius;
	}
	SH_GAME_API void ShpereCollider::DisplayArea(bool bDisplay)
	{
		bDisplayArea = bDisplay;
	}
	SH_GAME_API void ShpereCollider::OnPropertyChanged(const core::reflection::Property& prop)
	{
		if (prop.GetName() == "radius")
			SetRadius(radius);
	}
	SH_GAME_API void ShpereCollider::Update()
	{
		if (bDisplayArea)
		{
			if (debugRenderer != nullptr)
			{
				if (!debugRenderer->IsActive())
					debugRenderer->SetActive(true);
				debugRenderer->SetPosition(gameObject.transform->GetWorldPosition());
				float scale = radius / 0.5f;
				debugRenderer->SetScale(game::Vec3{ scale, scale, scale });
				debugRenderer->SetRotation(gameObject.transform->GetWorldRotation());
			}
		}
		else
		{
			if (debugRenderer != nullptr && debugRenderer->IsActive())
				debugRenderer->SetActive(false);
		}
	}
}//namespace