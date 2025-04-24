#pragma once
#include "Export.h"
#include "Util.h"

#include <array>

namespace sh::core
{
	/// @brief 객체 식별을 위한 UUID 클래스.
	/// @brief CHACHE_SIZE 만큼 객체를 미리 만들어두고 할당 한다.
	class UUID
	{
		friend struct std::hash<UUID>;
	private:
		std::array<uint32_t, 4> uuid{};
		std::string uuidStr;
	public:
		static constexpr std::size_t CHACHE_SIZE = 128;
	private:
		UUID() = default;
	public:
		/// @brief 문자열 UUID로 객체를 생성하는 함수
		/// @brief 올바르지 않은 형식이면 임의로 생성한다.
		/// @param str 문자열UUID
		SH_CORE_API UUID(std::string_view str);
		SH_CORE_API UUID(const UUID& other) noexcept;
		SH_CORE_API UUID(UUID&& other) noexcept;
		SH_CORE_API ~UUID() = default;

		SH_CORE_API auto operator=(const UUID& other) noexcept -> UUID&;
		SH_CORE_API auto operator=(UUID&& other) noexcept -> UUID&;
		SH_CORE_API bool operator==(const UUID& other) const noexcept;
		SH_CORE_API bool operator!=(const UUID& other) const noexcept;
		/// @brief 문자열로 변환하는 함수
		/// @return 16진수로 표기된 길이가 32인 문자열
		SH_CORE_API auto ToString() const -> const std::string&;

		/// @brief 새로운 UUID를 생성한다. 스레드 안전하다.
		SH_CORE_API static auto Generate() -> UUID;
	};
}//namespace

namespace std
{
	template<>
	struct hash<sh::core::UUID>
	{
		auto operator()(const sh::core::UUID& uuid) const -> std::size_t
		{
			std::hash<uint32_t> hash0;
			std::size_t result = hash0(uuid.uuid[0]);
			for (int i = 1; i < uuid.uuid.size(); ++i)
			{
				std::hash<uint32_t> hash;
				result = sh::core::Util::CombineHash(result, hash(uuid.uuid[i]));
			}
			return result;
		}
	};
}//namespace
