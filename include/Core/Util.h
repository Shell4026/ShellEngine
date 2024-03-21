#pragma once

#include <string>
namespace sh {
	class Util
	{
	public:
		static std::wstring U8StringToWstring(const std::string& u8str);
	};
}