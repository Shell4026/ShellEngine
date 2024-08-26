﻿#pragma once

#include "Export.h"
#include "Reflection.hpp"

#include <string>
#include <chrono>
#include <functional>
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
		/// @brief 해당 함수가 실행 되고 마치기까지 얼마나 걸리는지 반환한다.
		/// @param func 함수
		/// @return 걸린 시간(ms)
		SH_CORE_API static auto GetElapsedTime(const std::function<void()>& func) -> std::chrono::milliseconds;

		/// @brief 유니코드 형식의 string을 wstring으로 변환하는 함수.
		/// @param u8str utf-8 string
		/// @return wstring
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