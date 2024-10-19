#include "PCH.h"
#include "Component/BoxCollider.h"
#include "Component/RigidBody.h"

#include "Game/World.h"

namespace sh::game
{
	SH_GAME_API BoxCollider::BoxCollider(GameObject& owner) :
		Component(owner), size{ 1.0f, 1.0f, 1.0f }
	{
		shape = world.GetPhysWorld()->GetContext().createBoxShape(size / 2.0f);
	}
	SH_GAME_API BoxCollider::~BoxCollider()
	{
		SH_INFO("~BodCollider");
		world.GetPhysWorld()->GetContext().destroyBoxShape(shape);
	}

	SH_GAME_API void BoxCollider::OnDestroy()
	{
		SH_INFO("Destroy!");
		for (auto rb : rigidbodies)
		{
			rb->SetCollider(nullptr);
		}
	}

	auto BoxCollider::operator*() const -> reactphysics3d::BoxShape*
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
#if SH_EDITOR
	SH_GAME_API void BoxCollider::OnPropertyChanged(const core::reflection::Property& prop)
	{
		if(std::strcmp(prop.GetName(), "size") == 0)
			shape->setHalfExtents(size * 0.5f);
	}
#endif
}//namespace