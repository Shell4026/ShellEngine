#pragma once
#include "Export.h"
#include "SpinLock.h"

#include <string_view>
#include <unordered_map>
namespace sh::core
{
	/// @brief 비교가 빠른 문자열 클래스. 비교에 해시값을 사용한다. 스레드 안전하다.
	class Name
	{
	private:
		friend class std::hash<sh::core::Name>;

		static std::unordered_map<std::size_t, std::string> map;
		static SpinLock lock;

		const std::size_t hash;
	public:
		SH_CORE_API Name(std::string_view str);
		SH_CORE_API ~Name();

		SH_CORE_API auto operator==(const Name& other) const -> bool;
		SH_CORE_API auto operator!=(const Name& other) const -> bool;
		SH_CORE_API auto operator==(const std::string& str) const -> bool;
		SH_CORE_API auto operator!=(const std::string& str) const -> bool;

		SH_CORE_API auto ToString() const -> const std::string&;
	};
}//namespace

namespace std
{
	template<>
	class std::hash<sh::core::Name>
	{
	public:
		auto operator()(const sh::core::Name& name) const -> std::size_t
		{
			return name.hash;
		}
	};
}
