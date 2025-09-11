#include "Component/SphereCollider.h"
#include "Component/RigidBody.h"
#include "Component/DebugRenderer.h"

#include "Game/GameObject.h"
#include "Game/World.h"

#include "reactphysics3d/reactphysics3d.h"
namespace sh::game
{
	struct ShpereCollider::Impl
	{
		reactphysics3d::SphereShape* shape = nullptr;
	};

	SH_GAME_API ShpereCollider::ShpereCollider(GameObject& owner) :
		Collider(owner), radius(0.5)
	{
		impl = std::make_unique<Impl>();

		auto ctx = reinterpret_cast<reactphysics3d::PhysicsCommon*>(world.GetPhysWorld()->GetContext());
		impl->shape = ctx->createSphereShape(radius);
		if (IsEditor())
		{
			debugRenderer = gameObject.AddComponent<DebugRenderer>();
			render::Model* sphereModel = static_cast<render::Model*>(core::SObjectManager::GetInstance()->GetSObject(core::UUID{ "bbc4ef7ec45dce223297a224f8093f15" })); // Sphere Model
			debugRenderer->SetMesh(sphereModel->GetMeshes()[0]);
			debugRenderer->hideInspector = true;
		}
	}
	SH_GAME_API ShpereCollider::~ShpereCollider()
	{
		SH_INFO("~ShpereCollider");
		auto ctx = reinterpret_cast<reactphysics3d::PhysicsCommon*>(world.GetPhysWorld()->GetContext());
		ctx->destroySphereShape(impl->shape);
	}

	SH_GAME_API void ShpereCollider::OnDestroy()
	{
		if (debugRenderer != nullptr)
		{
			debugRenderer->Destroy();
			debugRenderer = nullptr;
		}
		Super::OnDestroy();
	}

	SH_GAME_API auto ShpereCollider::GetNative() const -> void*
	{
		return impl->shape;
	}

	SH_GAME_API void ShpereCollider::SetRadius(float r)
	{
		radius = r;
		if (r < 0.0001f)
			radius = 0.0001f;
		impl->shape->setRadius(radius);
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
		if (prop.GetName() == core::Util::ConstexprHash("radius"))
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