#pragma once
#include "Export.h"
#include <array>

namespace sh::core
{
	/// @brief 객체 식별을 위한 UUID 클래스
	/// @brief 필요 할 때마다 CHACHE_SIZE 만큼 객체를 만들어두고 할당 한다.
	class UUID
	{
	private:
		std::array<uint32_t, 4> uuid{};
	public:
		static constexpr std::size_t CHACHE_SIZE = 32;
	private:
		UUID() = default;
	public:
		SH_CORE_API UUID(const UUID& other) noexcept;
		SH_CORE_API ~UUID() = default;

		SH_CORE_API auto operator=(const UUID& other) noexcept -> UUID&;
		SH_CORE_API bool operator==(const UUID& other) const noexcept;
		SH_CORE_API bool operator!=(const UUID& other) const noexcept;
		/// @brief 문자열로 변환하는 함수
		/// @return 16진수로 표기된 길이가 32인 문자열
		SH_CORE_API auto ToString() const -> std::string;

		SH_CORE_API static auto Generate() -> UUID;
	};
}//namespace