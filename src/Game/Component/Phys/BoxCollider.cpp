#include "Component/Phys/BoxCollider.h"
#include "Component/Phys/RigidBody.h"
#include "Component/Render/DebugRenderer.h"

#include "Game/GameObject.h"
#include "Game/World.h"

#include "reactphysics3d/reactphysics3d.h"
namespace sh::game
{
	struct BoxCollider::Impl
	{
		reactphysics3d::BoxShape* shape = nullptr;
	};

	SH_GAME_API BoxCollider::BoxCollider(GameObject& owner) :
		Collider(owner), size{ 1.0f, 1.0f, 1.0f }
	{
		impl = std::make_unique<Impl>();

		auto ctx = reinterpret_cast<reactphysics3d::PhysicsCommon*>(world.GetPhysWorld()->GetContext());
		Vec3 halfSize = size / 2.0f;
		impl->shape = ctx->createBoxShape(reactphysics3d::Vector3{ halfSize.x, halfSize.y, halfSize.z });

		if (IsEditor())
		{
			debugRenderer = gameObject.AddComponent<DebugRenderer>();
			render::Model* cubeModel = static_cast<render::Model*>(core::SObjectManager::GetInstance()->GetSObject(core::UUID{ "bbc4ef7ec45dce223297a224f8093f16" })); // Cube Model
			debugRenderer->SetMesh(cubeModel->GetMeshes()[0]);
			debugRenderer->hideInspector = true;
		}
	}
	SH_GAME_API BoxCollider::~BoxCollider()
	{
		SH_INFO("~BoxCollider");
		auto ctx = reinterpret_cast<reactphysics3d::PhysicsCommon*>(world.GetPhysWorld()->GetContext());
		ctx->destroyBoxShape(impl->shape);
	}

	SH_GAME_API void BoxCollider::OnDestroy()
	{
		if (debugRenderer)
		{
			debugRenderer->Destroy();
			debugRenderer = nullptr;
		}
		Super::OnDestroy();
	}

	SH_GAME_API auto BoxCollider::GetNative() const -> void*
	{
		return impl->shape;
	}

	SH_GAME_API void BoxCollider::SetSize(const Vec3& size)
	{
		this->size = size;
		Vec3 halfSize = size / 2.0f;
		impl->shape->setHalfExtents(reactphysics3d::Vector3{ halfSize.x, halfSize.y, halfSize.z });
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
		Super::OnPropertyChanged(prop);
		if (prop.GetName() == core::Util::ConstexprHash("size"))
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