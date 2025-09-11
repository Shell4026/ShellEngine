#include "Component/Collider.h"
#include "Component/RigidBody.h"

#include "reactphysics3d/reactphysics3d.h"

#include <algorithm>
namespace sh::game
{
	Collider::Collider(GameObject& owner) :
		Component(owner)
	{
		canPlayInEditor = true;
	}
	SH_GAME_API void Collider::SetCollisionTag(phys::Tag tag)
	{
		this->tag = tag;

		bool bRemove = false;
		std::vector<bool> removeIdx(handles.size(), false);
		for (int i = 0; i < handles.size(); ++i)
		{
			auto& handle = handles[i];
			if (!handle.rb.IsValid())
			{
				removeIdx[i] = true;
				bRemove = true;
				continue;
			}
			reinterpret_cast<reactphysics3d::Collider*>(handle.nativeCollider)->setCollisionCategoryBits(tag);
		}
		if (bRemove)
		{
			int idx = 0;
			handles.erase(std::remove_if(handles.begin(), handles.end(), [&](const Handle& handle) {return removeIdx[idx++]; }), handles.end());
		}
	}
	SH_GAME_API auto Collider::GetCollisionTag() const -> phys::Tag
	{
		return tag;
	}
	SH_GAME_API void Collider::SetAllowCollisions(phys::Tagbit tags)
	{
		allowed = tags;
		bool bRemove = false;
		std::vector<bool> removeIdx(handles.size(), false);
		for (int i = 0; i < handles.size(); ++i)
		{
			auto& handle = handles[i];
			if (!handle.rb.IsValid())
			{
				removeIdx[i] = true;
				bRemove = true;
				continue;
			}
			auto bodyPtr = reinterpret_cast<reactphysics3d::RigidBody*>(handle.rb->GetNativeHandle());
			reinterpret_cast<reactphysics3d::Collider*>(handle.nativeCollider)->setCollideWithMaskBits(tags);
		}
		if (bRemove)
		{
			int idx = 0;
			handles.erase(std::remove_if(handles.begin(), handles.end(), [&](const Handle& handle) {return removeIdx[idx++]; }), handles.end());
		}
	}
	SH_GAME_API auto Collider::GetAllowCollisions() const -> phys::Tagbit
	{
		return allowed;
	}
	Collider::Handle::~Handle()
	{
	}
}//namespace