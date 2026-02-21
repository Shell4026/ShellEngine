#pragma once
#include "Physics/ContactPoint.h"

#include "Core/IEvent.h"
#include "Core/Reflection.hpp"

#include <functional>
#include <cstdint>
namespace sh::phys
{
	struct PhysicsEvent : core::IEvent
	{
		auto GetTypeHash() const -> std::size_t
		{
			return core::reflection::GetType<PhysicsEvent>().hash;
		}

		void* rigidBody1Handle = nullptr;
		void* rigidBody2Handle = nullptr;
		void* collider1Handle = nullptr;
		void* collider2Handle = nullptr;

		enum class Type
		{
			CollisionEnter,
			CollisionStay,
			CollisionExit,
			TriggerEnter,
			TriggerExit
		} type;

		uint32_t contactCount = 0;
		std::function<void(ContactPoint&, uint32_t)> getContactPointFn;
	};
}//namespace