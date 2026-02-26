#include "Collision.h"
#include "Component/Phys/Collider.h"

namespace sh::game
{
	Collision::Collision(const std::array<phys::ContactPoint, ARRAY_SIZE>& contactPoints)
	{
		contacts = contactPoints;
	}
	Collision::Collision(std::vector<phys::ContactPoint>&& contactPoints)
	{
		contacts = std::move(contactPoints);
	}
	Collision::Collision(Collision&& other) noexcept :
		collider(other.collider),
		contactCount(other.contactCount),
		contacts(std::move(other.contacts))
	{
	}
	SH_GAME_API auto Collision::operator=(Collision&& other) noexcept -> Collision&
	{
		collider = other.collider;
		contactCount = other.contactCount;
		contacts = std::move(other.contacts);
		return *this;
	}
	SH_GAME_API void Collision::PushReferenceObjects(core::GarbageCollection& gc)
	{
		gc.PushReferenceObject(collider);
	}
	SH_GAME_API auto Collision::GetContactPoint(uint32_t idx) const -> const phys::ContactPoint&
	{
		if (contacts.index() == 0)
			return std::get<0>(contacts)[idx];
		else
			return std::get<1>(contacts)[idx];
	}
}//namespace