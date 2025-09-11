#include "Component/CapsuleCollider.h"
#include "Component/RigidBody.h"
#include "Component/DebugRenderer.h"

#include "Game/GameObject.h"
#include "Game/World.h"

#include "Render/Mesh/CapsuleMesh.h"

#include "reactphysics3d/reactphysics3d.h"
namespace sh::game
{
	struct CapsuleCollider::Impl
	{
		reactphysics3d::CapsuleShape* shape = nullptr;
	};

	SH_GAME_API CapsuleCollider::CapsuleCollider(GameObject& owner) :
		Collider(owner)
	{
		impl = std::make_unique<Impl>();

		auto ctx = reinterpret_cast<reactphysics3d::PhysicsCommon*>(world.GetPhysWorld()->GetContext());
		impl->shape = ctx->createCapsuleShape(radius, height);

#if SH_EDITOR
		debugRenderer = gameObject.AddComponent<DebugRenderer>();
		render::Mesh* capsuleMesh = static_cast<render::Mesh*>(core::SObject::Create<render::CapsuleMesh>(radius, height));
		capsuleMesh->Build(*world.renderer.GetContext());
		debugRenderer->SetMesh(capsuleMesh);
		debugRenderer->hideInspector = true;
#endif
	}
	SH_GAME_API CapsuleCollider::~CapsuleCollider()
	{
		SH_INFO("~CapsuleCollider");
		auto ctx = reinterpret_cast<reactphysics3d::PhysicsCommon*>(world.GetPhysWorld()->GetContext());
		ctx->destroyCapsuleShape(impl->shape);
	}

	SH_GAME_API void CapsuleCollider::OnDestroy()
	{
		if (debugRenderer != nullptr)
		{
			debugRenderer->Destroy();
			debugRenderer = nullptr;
		}
		Super::OnDestroy();
	}

	SH_GAME_API void CapsuleCollider::Awake()
	{
	}

	SH_GAME_API auto CapsuleCollider::GetNative() const -> void*
	{
		return impl->shape;
	}

	SH_GAME_API void CapsuleCollider::SetRadius(float r)
	{
		if (r < 0.0001f)
			r = 0.0001f;
		radius = r;
		impl->shape->setRadius(radius);

		render::Mesh* capsuleMesh = static_cast<render::Mesh*>(core::SObject::Create<render::CapsuleMesh>(radius, height));
		capsuleMesh->Build(*world.renderer.GetContext());
		debugRenderer->SetMesh(capsuleMesh);
	}
	SH_GAME_API auto CapsuleCollider::GetRadius() const -> float
	{
		return radius;
	}
	SH_GAME_API void CapsuleCollider::SetHeight(float h)
	{
		if (h < 0.0001f)
			h = 0.0001f;
		height = h;
		impl->shape->setHeight(height);

		render::Mesh* capsuleMesh = static_cast<render::Mesh*>(core::SObject::Create<render::CapsuleMesh>(radius, height));
		capsuleMesh->Build(*world.renderer.GetContext());
		debugRenderer->SetMesh(capsuleMesh);
	}
	SH_GAME_API auto CapsuleCollider::GetHeight() const -> float
	{
		return height;
	}
	SH_GAME_API void CapsuleCollider::DisplayArea(bool bDisplay)
	{
		bDisplayArea = bDisplay;
	}
	SH_GAME_API void CapsuleCollider::OnPropertyChanged(const core::reflection::Property& prop)
	{
		if (prop.GetName() == core::Util::ConstexprHash("radius"))
			SetRadius(radius);
		else if (prop.GetName() == core::Util::ConstexprHash("height"))
			SetHeight(height);
	}
	SH_GAME_API void CapsuleCollider::Update()
	{
		if (bDisplayArea)
		{
			if (debugRenderer != nullptr)
			{
				if (!debugRenderer->IsActive())
					debugRenderer->SetActive(true);
				debugRenderer->SetPosition(gameObject.transform->GetWorldPosition());
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