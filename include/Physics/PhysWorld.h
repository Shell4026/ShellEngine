#pragma once
#include "Export.h"
#include "Ray.h"
#include "HitPoint.h"
#include "CollisionTag.hpp"

#include "Core/EventBus.h"
#include "Core/IEvent.h"
#include "Core/Reflection.hpp"

#include <glm/vec3.hpp>

#include <memory>
#include <optional>
#include <functional>
#include <unordered_map>
namespace sh::phys
{
	class PhysWorld
	{
	public:
		using ContextHandle = void*;
		using PhysicsWorldHandle = void*;
		struct PhysicsEvent : core::IEvent
		{
			auto GetTypeHash() const -> std::size_t
			{
				return core::reflection::GetType<PhysicsEvent>().hash;
			}

			void* rigidBody1Handle = nullptr;
			void* rigidBody2Handle = nullptr;
			enum class Type
			{
				CollisionEnter,
				CollisionStay,
				CollisionExit,
				TriggerEnter,
				TriggerStay,
				TriggerExit
			} type;
		};
	public:
		SH_PHYS_API PhysWorld();
		SH_PHYS_API PhysWorld(PhysWorld&& other) noexcept;
		SH_PHYS_API ~PhysWorld();

		SH_PHYS_API void Clean();

		SH_PHYS_API void Update(float deltaTime);

		SH_PHYS_API auto RayCastHit(const Ray& ray, Tagbit allowedTag = 0xffff) const -> bool;
		SH_PHYS_API auto RayCast(const Ray& ray, Tagbit allowedTag = 0xffff) const -> std::vector<HitPoint>;

		SH_PHYS_API auto GetContext() const -> ContextHandle;
		SH_PHYS_API auto GetNative() const -> PhysicsWorldHandle;

		SH_PHYS_API void SetGravity(const glm::vec3& gravity);
		SH_PHYS_API auto GetGravity() const -> glm::vec3;
	public:
		core::EventBus bus;
	private:
		struct Impl;
		
		std::unique_ptr<Impl> impl;
	};
}//namespace