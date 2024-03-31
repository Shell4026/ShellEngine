#pragma once

#include <string>
namespace sh::core {
	class Util
	{
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
	};
}