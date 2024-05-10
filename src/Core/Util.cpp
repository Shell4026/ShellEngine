#include "Util.h"
#include "SObject.h"

namespace sh::core {
	auto Util::U8StringToWstring(const std::string& u8str) -> std::wstring
	{
		std::wstring result;
		for (int i = 0; i < u8str.size();)
		{
			unsigned char c0 = u8str[i];
			if (c0 >> 3 == 0b11110) //4byte utf
			{
				c0 = (c0 & 0b00000111);
				char c1 = (u8str[i + 1] & 0b00111111);
				char c2 = (u8str[i + 2] & 0b00111111);
				char c3 = (u8str[i + 3] & 0b00111111);
				int unicode = (c3 | c2 << 6) | ((c2 >> 2 | c1 << 4) << 8) | ((c1 >> 4 | c0 << 2) << 16);
				//UTF-16 incoding
				wchar_t high = 0xD800 + ((unicode - 0x10000) / 0x400);
				wchar_t low = 0xDC00 + ((unicode - 0x10000) % 0x400);
				result += high;
				result += low;
				i += 4;
			}
			else if (c0 >> 4 == 0b1110) //3byte utf
			{
				c0 = (c0 & 0b00001111);
				char c1 = (u8str[i + 1] & 0b00111111);
				char c2 = (u8str[i + 2] & 0b00111111);
				result += (c2 | c1 << 6) | ((c1 >> 2 | c0 << 4) << 8);
				i += 3;
			}
			else if (c0 >> 5 == 0b110) //2byte utf
			{
				c0 = (c0 & 0b00011111);
				char c1 = (u8str[i + 1] & 0b00111111);
				result += (c1 | c0 << 6) | (c0 >> 2 << 8);
				i += 2;
			}
			else if (c0 >> 7 == 0b0) //1byte utf
			{
				result += c0;
				++i;
			}
		}
		return result;
	}

	bool IsValid(SObject* obj)
	{
		if (obj == nullptr)
			return false;
		return !obj->IsPendingKill();
	}
}