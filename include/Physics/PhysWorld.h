#pragma once
#include "Export.h"
#include "Ray.h"
#include "HitPoint.h"

#include <glm/vec3.hpp>

#include <memory>
#include <optional>
namespace sh::phys
{
	class PhysWorld
	{
	public:
		SH_PHYS_API PhysWorld();
		SH_PHYS_API PhysWorld(PhysWorld&& other) noexcept;
		SH_PHYS_API ~PhysWorld();

		SH_PHYS_API void Clean();

		SH_PHYS_API void Update(float deltaTime);

		SH_PHYS_API auto RayCastHit(const Ray& ray) const -> bool;
		SH_PHYS_API auto RayCast(const Ray& ray) const -> std::optional<HitPoint>;

		SH_PHYS_API auto GetContext() const -> void*;
		SH_PHYS_API auto GetNative() const -> void*;

		SH_PHYS_API void SetGravity(const glm::vec3& gravity);
		SH_PHYS_API auto GetGravity() const -> glm::vec3;
	private:
		struct Impl;
		
		std::unique_ptr<Impl> impl;
	};
}//namespace