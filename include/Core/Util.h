#pragma once

#include <string>
namespace sh::core {
	class Util
	{
	public:
		enum class Platform {
			Windows,
			Linux,
			Unknown
		};
	public:
		static auto U8StringToWstring(const std::string& u8str)->std::wstring;
		static constexpr bool IsDebug()
		{
#ifdef SH_DEBUG
			return true;
#else
			return false;
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