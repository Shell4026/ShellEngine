#pragma once

#include "Export.h"
#include "Reflection.hpp"

#include <string>
namespace sh::core {
	class SObject;

	class Util
	{
	public:
		enum class Platform {
			Windows,
			Linux,
			Unknown
		};
	public:
		//유니코드 형식의 string을 wstring으로 변환하는 함수.
		SH_CORE_API static auto U8StringToWstring(const std::string& u8str)->std::wstring;
		SH_CORE_API static constexpr bool IsDebug()
		{
#ifdef NDEBUG
			return false;
#else
			return true;
#endif
		}
		static constexpr auto GetPlatform()-> Platform {
#if _WIN32
			return Platform::Windows;
#elif __linux__
			return Platform::Linux;
#else
			return Platform::Unknown;
#endif
		}
	};

	SH_CORE_API bool IsValid(SObject* obj);
}