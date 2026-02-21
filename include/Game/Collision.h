#pragma once
#include "Export.h"

#include "Physics/ContactPoint.h"

#include <array>
#include <vector>
#include <variant>
namespace sh::game
{
	class Collider;
	struct Collision
	{
		static constexpr std::size_t ARRAY_SIZE = 6;

		SH_GAME_API Collision(const std::array<phys::ContactPoint, ARRAY_SIZE>& contactPoints);
		SH_GAME_API Collision(std::vector<phys::ContactPoint>&& contactPoints);
		SH_GAME_API Collision(Collision&& other) noexcept;
		SH_GAME_API auto operator=(Collision&& other) noexcept -> Collision&;

		SH_GAME_API auto GetContactPoint(uint32_t idx) const -> const phys::ContactPoint&;

		Collider* collider = nullptr;
		uint32_t contactCount = 0;
	private:
		std::variant<std::array<phys::ContactPoint, ARRAY_SIZE>, std::vector<phys::ContactPoint>> contacts;
	};
}//namespace