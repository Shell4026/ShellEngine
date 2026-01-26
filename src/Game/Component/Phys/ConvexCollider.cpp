#include "Component/Phys/ConvexCollider.h"
#include "Component/Phys/RigidBody.h"
#include "Component/Render/DebugRenderer.h"
#include "Component/Render/MeshRenderer.h"

#include "Render/Mesh.h"

#include "Game/GameObject.h"
#include "Game/World.h"

#include "reactphysics3d/reactphysics3d.h"
namespace sh::game
{
	struct ConvexCollider::Impl
	{
		reactphysics3d::ConvexMesh* convexMesh = nullptr;
		reactphysics3d::ConvexMeshShape* shape = nullptr;
	};

	SH_GAME_API ConvexCollider::ConvexCollider(GameObject& owner) :
		Collider(owner)
	{
		impl = std::make_unique<Impl>();
//#if SH_EDITOR
//		debugRenderer = gameObject.AddComponent<DebugRenderer>();
//		render::Model* cubeModel = static_cast<render::Model*>(core::SObjectManager::GetInstance()->GetSObject(core::UUID{ "bbc4ef7ec45dce223297a224f8093f16" })); // Cube Model
//		debugRenderer->SetMesh(cubeModel->GetMeshes()[0]);
//		debugRenderer->hideInspector = true;
//#endif
	}
	SH_GAME_API ConvexCollider::~ConvexCollider()
	{
		SH_INFO("~ConvexCollider");
		DestroyShape();
	}

	SH_GAME_API void ConvexCollider::Start()
	{
		CreateShape();
	}

	SH_GAME_API void ConvexCollider::OnDestroy()
	{
		//if (debugRenderer)
		//{
		//	debugRenderer->Destroy();
		//	debugRenderer = nullptr;
		//}
		Super::OnDestroy();
	}

	SH_GAME_API auto ConvexCollider::GetNative() const -> void*
	{
		return impl->shape;
	}
	//SH_GAME_API void ConvexCollider::DisplayArea(bool bDisplay)
	//{
	//	bDisplayArea = bDisplay;
	//}
	SH_GAME_API void ConvexCollider::OnPropertyChanged(const core::reflection::Property& prop)
	{
		Super::OnPropertyChanged(prop);
	}
	SH_GAME_API void ConvexCollider::Update()
	{
		if (lastMesh != mesh)
		{
			DestroyShape();
			CreateShape();
		}
	//	if (bDisplayArea)
	//	{
	//		if (debugRenderer != nullptr)
	//		{
	//			if (!debugRenderer->IsActive())
	//				debugRenderer->SetActive(true);
	//			debugRenderer->SetPosition(gameObject.transform->GetWorldPosition());
	//			debugRenderer->SetScale(size);
	//			debugRenderer->SetRotation(gameObject.transform->GetWorldRotation());
	//		}
	//	}
	//	else
	//	{
	//		if (debugRenderer != nullptr && debugRenderer->IsActive())
	//			debugRenderer->SetActive(false);
	//	}
	}
	void ConvexCollider::CreateShape()
	{
		const MeshRenderer* const renderer = gameObject.GetComponent<MeshRenderer>();
		if (renderer == nullptr)
			return;
		mesh = renderer->GetMesh();
		if (!core::IsValid(mesh))
			return;
		lastMesh = mesh;

		auto ctx = reinterpret_cast<reactphysics3d::PhysicsCommon*>(world.GetPhysWorld()->GetContext());

		const auto& meshVerts = mesh->GetVertex();
		const reactphysics3d::VertexArray verts{ meshVerts.data(), sizeof(render::Mesh::Vertex), static_cast<uint32_t>(meshVerts.size()), reactphysics3d::VertexArray::DataType::VERTEX_FLOAT_TYPE };
		std::vector<reactphysics3d::Message> msgs;
		impl->convexMesh = ctx->createConvexMesh(verts, msgs);
		if (!msgs.empty())
		{
			for (auto& msg : msgs)
			{
				if (msg.type == reactphysics3d::Message::Type::Error)
				{
					SH_ERROR_FORMAT("{}", msg.text);
					return;
				}
				else
					SH_INFO_FORMAT("{}", msg.text);
			}
		}
		const auto& objScale = gameObject.transform->GetWorldScale();
		impl->shape = ctx->createConvexMeshShape(impl->convexMesh, { objScale.x, objScale.y, objScale.z });
	}
	void ConvexCollider::DestroyShape()
	{
		auto ctx = reinterpret_cast<reactphysics3d::PhysicsCommon*>(world.GetPhysWorld()->GetContext());
		if (impl->shape != nullptr)
			ctx->destroyConvexMeshShape(impl->shape);
		if (impl->convexMesh != nullptr)
			ctx->destroyConvexMesh(impl->convexMesh);
		impl->shape = nullptr;
		impl->convexMesh = nullptr;
	}
}//namespace