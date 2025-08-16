#pragma once
#include "Export.h"
#include "Ray.h"

#include <memory>
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

		SH_PHYS_API auto GetContext() const -> void*;
		SH_PHYS_API auto GetNative() const -> void*;
	private:
		struct Impl;
		
		std::unique_ptr<Impl> impl;
	};
}//namespace