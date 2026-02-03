#pragma once
#include "Export.h"

#include "Core/SObject.h"

#include <vector>
#include <cstdint>
namespace sh::game
{
	class BinaryObject : public core::SObject
	{
		SCLASS(BinaryObject)
	public:
		SH_GAME_API auto Serialize() const -> core::Json override;
		SH_GAME_API void Deserialize(const core::Json& json) override;

		SH_GAME_API auto operator=(const BinaryObject& other) -> BinaryObject&;
		SH_GAME_API auto operator=(BinaryObject&& other) noexcept -> BinaryObject&;
	public:
		std::vector<uint8_t> data;
	};
}//namespace