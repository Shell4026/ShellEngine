#pragma once
#include "Export.h"

#include <string_view>
#include <unordered_map>
#include <shared_mutex>
namespace sh::core
{
	/// @brief 비교가 빠른 문자열 클래스. 비교에 해시값을 사용한다. 
	/// @brief 같은 문자열은 같은 주소를 가르키며 생성 시 스레드 안전하다.
	class Name
	{
	private:
		friend struct std::hash<sh::core::Name>;

		SH_CORE_API static std::unordered_map<std::size_t, std::string> map;
		SH_CORE_API static std::shared_mutex mu;

		std::size_t hash;
	public:
		SH_CORE_API Name(std::string_view str);
		SH_CORE_API Name(const Name& other) noexcept;
		SH_CORE_API Name(Name&& other) noexcept;
		SH_CORE_API ~Name();

		SH_CORE_API auto operator=(const Name& other) noexcept -> Name&;
		SH_CORE_API auto operator=(Name&& other) noexcept -> Name&;
		SH_CORE_API auto operator==(const Name& other) const -> bool;
		SH_CORE_API auto operator!=(const Name& other) const -> bool;
		SH_CORE_API auto operator==(const std::string_view str) const -> bool;
		SH_CORE_API auto operator!=(const std::string_view str) const -> bool;

		SH_CORE_API operator const std::string& () const;
		SH_CORE_API auto ToString() const -> const std::string&;
	};
}//namespace

namespace std
{
	template<>
	struct std::hash<sh::core::Name>
	{
		auto operator()(const sh::core::Name& name) const -> std::size_t
		{
			return name.hash;
		}
	};

	static auto operator==(std::string_view str, const sh::core::Name& name) -> bool
	{
		return name == str;
	}
	static auto operator!=(std::string_view str, const sh::core::Name& name) -> bool
	{
		return name != str;
	}
}
