#include "Component/BoxCollider.h"
#include "Component/RigidBody.h"
#include "Component/DebugRenderer.h"

#include "Game/GameObject.h"
#include "Game/World.h"

namespace sh::game
{
	SH_GAME_API BoxCollider::BoxCollider(GameObject& owner) :
		Collider(owner), size{ 1.0f, 1.0f, 1.0f }
	{
		shape = world.GetPhysWorld()->GetContext().createBoxShape(size / 2.0f);
#if SH_EDITOR
		debugRenderer = gameObject.AddComponent<DebugRenderer>();
		debugRenderer->SetMesh(world.models.GetResource("CubeModel")->GetMeshes()[0]);
		debugRenderer->hideInspector = true;
#endif
	}
	SH_GAME_API BoxCollider::~BoxCollider()
	{
		SH_INFO("~BoxCollider");
		world.GetPhysWorld()->GetContext().destroyBoxShape(shape);
	}

	SH_GAME_API void BoxCollider::Destroy()
	{
		Super::Destroy();
		if (debugRenderer)
		{
			debugRenderer->Destroy();
			debugRenderer = nullptr;
		}
	}

	SH_GAME_API void BoxCollider::OnDestroy()
	{
		SH_INFO("Destroy!");
		for (auto rb : rigidbodies)
		{
			rb->SetCollider(nullptr);
		}
	}

	SH_GAME_API auto BoxCollider::GetCollisionShape() const -> reactphysics3d::CollisionShape*
	{
		return shape;
	}

	SH_GAME_API void BoxCollider::SetSize(const Vec3& size)
	{
		this->size = size;
		shape->setHalfExtents(size * 0.5f);
	}
	SH_GAME_API auto BoxCollider::GetSize() const -> const Vec3&
	{
		return size;
	}
	SH_GAME_API void BoxCollider::DisplayArea(bool bDisplay)
	{
		bDisplayArea = bDisplay;
	}
	SH_GAME_API void BoxCollider::OnPropertyChanged(const core::reflection::Property& prop)
	{
		if (prop.GetName() == "size")
			SetSize(size);
	}
	SH_GAME_API void BoxCollider::Update()
	{
		if (bDisplayArea)
		{
			if (debugRenderer != nullptr)
			{
				if (!debugRenderer->IsActive())
					debugRenderer->SetActive(true);
				debugRenderer->SetPosition(gameObject.transform->GetWorldPosition());
				debugRenderer->SetScale(size);
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