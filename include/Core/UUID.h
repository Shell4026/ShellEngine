#pragma once
#include "Export.h"
#include "Util.h"

#include <array>
#include <string_view>
namespace sh::core
{
	/// @brief 객체 식별을 위한 UUID 클래스.
	class UUID
	{
		friend struct std::hash<UUID>;
	public:
		/// @brief 문자열 UUID로 객체를 생성하는 함수
		/// @brief 올바르지 않은 형식이면 0으로 초기화 된 UUID를 생성한다.
		/// @param str 문자열UUID
		SH_CORE_API UUID(std::string_view str);
		SH_CORE_API UUID(const std::array<uint32_t, 4>& uuid);
		SH_CORE_API UUID(const UUID& other) noexcept;
		SH_CORE_API ~UUID() = default;

		SH_CORE_API auto operator=(const UUID& other) noexcept -> UUID&;
		SH_CORE_API auto operator==(const UUID& other) const noexcept -> bool;
		SH_CORE_API auto operator==(std::string_view str) const noexcept -> bool;
		SH_CORE_API auto operator!=(const UUID& other) const noexcept -> bool;
		SH_CORE_API auto operator!=(std::string_view str) const noexcept -> bool;
		SH_CORE_API      operator const std::array<uint32_t, 4>&() const { return uuid; }
		/// @brief 문자열로 변환하는 함수
		/// @return 16진수로 표기된 길이가 32인 문자열
		SH_CORE_API auto ToString() const -> std::string;
		SH_CORE_API auto GetRawData() const -> const std::array<uint32_t, 4>&;
		SH_CORE_API auto IsEmpty() const -> bool;

		/// @brief 새로운 UUID를 생성한다.
		SH_CORE_API static auto Generate() -> UUID;
		/// @brief 0으로 초기화 된 UUID를 생성한다.
		SH_CORE_API static auto GenerateEmptyUUID() -> UUID;
	private:
		UUID() = default;
	private:
		std::array<uint32_t, 4> uuid{};
	};
}//namespace

namespace std
{
	template<>
	struct hash<sh::core::UUID>
	{
		auto operator()(const sh::core::UUID& uuid) const noexcept -> std::size_t
		{
			std::hash<uint32_t> hasher;
			std::size_t result = hasher(uuid.uuid[0]);
			for (int i = 1; i < uuid.uuid.size(); ++i)
				result = sh::core::Util::CombineHash(result, hasher(uuid.uuid[i]));
			return result;
		}
	};
}//namespace
