#pragma once
#include "Export.h"

#include <string>
#include <mutex>
#include <unordered_map>
namespace sh::core
{
	/// @brief 비교가 빠른 문자열 클래스. 스레드 안전하다.
	class Name
	{
	private:
		SH_CORE_API static std::unordered_map<std::string, int> dic;
		SH_CORE_API static std::mutex mu;
		friend class std::hash<sh::core::Name>;

		const std::string* ptr;
	public:
		SH_CORE_API Name(const std::string& str);
		SH_CORE_API Name(const char* str);
		SH_CORE_API ~Name();

		SH_CORE_API auto operator==(const Name& other) const -> bool;
		SH_CORE_API auto operator==(const std::string& str) const -> bool;
		SH_CORE_API auto operator!=(const Name& other) const -> bool;
		SH_CORE_API auto operator!=(const std::string& str) const -> bool;
		SH_CORE_API      operator const std::string& () const;

		SH_CORE_API auto GetString() const -> const std::string&;
	};
}//namespace

namespace std
{
	template<>
	struct std::hash<sh::core::Name>
	{
		auto operator()(const sh::core::Name& name) const -> std::size_t
		{
			std::hash<const void*> hasher{};
			return hasher(reinterpret_cast<const void*>(name.ptr));
		}
	};
}
