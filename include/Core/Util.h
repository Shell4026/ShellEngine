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

		/// @brief 두 해시를 합쳐 새로운 해시로 만드는 함수
		/// @param v1 해시1
		/// @param v2 해시2
		/// @return 합쳐진 새로운 해시
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
		/// @brief 주어진 값을 정렬 값에 가까운 배수로 올림 하는 함수. 메모리 정렬에 사용된다.
		/// 예시: (20, 16) => 32
		/// @param value 입력 값
		/// @param alignment 정렬 값
		/// @return 입력값에서 올림된 정렬 값의 배수
		SH_CORE_API static auto AlignTo(uint32_t value, uint32_t alignment) -> uint32_t;
	};

	/// @brief 해당 SObject가 nullptr이거나 앞으로 지워질 객체인지 검증 하는 함수.
	/// @param obj SObject 포인터
	/// @return 유효하면 true, 아니면 false
	SH_CORE_API bool IsValid(const SObject* obj);
}