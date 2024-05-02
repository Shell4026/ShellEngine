#pragma once

#include "Export.h"
#include "Reflaction.hpp"

#include <string>
namespace sh::core {
	class SH_CORE_API Util
	{
	public:
		enum class Platform {
			Windows,
			Linux,
			Unknown
		};
	public:
		//유니코드 형식의 string을 wstring으로 변환하는 함수.
		static auto U8StringToWstring(const std::string& u8str)->std::wstring;
		static constexpr bool IsDebug()
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
}