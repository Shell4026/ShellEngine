#pragma once

#include "Export.h"

#include <glm/mat2x2.hpp>
#include <glm/mat4x4.hpp>

#include <string>
#include <chrono>
#include <functional>
#include <random>
#include <cstdint>

namespace sh::core 
{
	class SObject;

	class Util
	{
	private:
		inline static std::random_device seed{};
		inline static std::mt19937 gen{ seed() };
	public:
		enum class Platform {
			Windows,
			Linux,
			Unknown
		};
	public:
		/// @brief 해당 함수가 실행 되고 마치기까지 얼마나 걸리는지 반환한다.
		/// @param func 함수
		/// @tparam std::chrono 시간 단위
		/// @return 걸린 시간
		template<typename T>
		static auto GetElapsedTime(const std::function<void()>& func) -> T
		{
			auto start = std::chrono::high_resolution_clock::now();
			func();
			auto end = std::chrono::high_resolution_clock::now();
			return std::chrono::duration_cast<T>(end - start);
		}

		/// @brief 유니코드 형식의 string을 wstring으로 변환하는 함수.
		/// @param u8str utf-8 string
		/// @return wstring
		SH_CORE_API static auto U8StringToWstring(const std::string& u8str)->std::wstring;

		/// @brief 디버그 환경인지 판별하는 함수
		/// @return 맞으면 true 아니면 false
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

		/// @brief 컴파일 시간에 FNV-1a 해시 알고리즘을 사용해 해시 값을 반환 한다.
		/// @param str 문자열
		/// @return 해시 
		constexpr static auto ConstexprHash(std::string_view str) -> std::size_t
		{
			constexpr std::size_t FNVOffsetBasis = 14695981039346656037ULL;
			constexpr std::size_t FNVPrime = 1099511628211ULL;

			std::size_t hash = FNVOffsetBasis;
			for (char c : str)
			{
				hash ^= static_cast<std::size_t>(c);
				hash *= FNVPrime;
			}
			return hash;
		}

		SH_CORE_API static auto RandomRange(uint32_t min, uint32_t max) -> uint32_t;
		SH_CORE_API static auto RandomRange(int min, int max) -> int;
		SH_CORE_API static auto RandomRange(float min, float max) -> float;
		SH_CORE_API static auto RandomRange(double min, double max) -> double;

		/// @brief 문자열에서 공백을 제거하고 _로 대체하는 함수
		/// @param str 문자열
		/// @return 공백이 대체된 문자열
		SH_CORE_API static auto ReplaceSpaceString(const std::string& str) -> std::string;
		/// @brief 바이트 배열을 리틀 엔디안 워드(4바이트) 배열로 바꾸는 함수.
		/// @param bytes 바이트 배열
		/// @return 워드 배열
		SH_CORE_API static auto ConvertByteToWord(const std::vector<uint8_t>& bytes) -> std::vector<uint32_t>;

		SH_CORE_API static auto ConvertMat2ToMat4(const glm::mat2& mat) -> glm::mat4;
		SH_CORE_API static auto ConvertMat4ToMat2(const glm::mat4& mat) -> glm::mat2;
		SH_CORE_API static auto ConvertMat3ToMat4(const glm::mat3& mat) -> glm::mat4;
		SH_CORE_API static auto ConvertMat4ToMat3(const glm::mat4& mat) -> glm::mat3;
	};

	/// @brief 해당 SObject가 nullptr이거나 앞으로 지워질 객체인지 검증 하는 함수.
	/// @param obj SObject 포인터
	/// @return 유효하면 true, 아니면 false
	SH_CORE_API bool IsValid(const SObject* obj);
}