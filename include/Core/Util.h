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

		static inline auto CombineHash(size_t v1, size_t v2) -> size_t
		{
			return v1 ^= v2 + 0x9e3779b9 + (v1 << 6) + (v1 >> 2);
		}

		struct PairHash {
			template <class T1, class T2>
			std::size_t operator () (const std::pair<T1, T2>& p) const {
				auto h1 = std::hash<T1>{}(p.first);
				auto h2 = std::hash<T2>{}(p.second);

				return CombineHash(h1, h2);
			}
		};
	};

	SH_CORE_API bool IsValid(SObject* obj);
}