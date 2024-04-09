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

		/// \brief 빠른 다운 캐스팅.
		///
		/// 둘 다 SCLASS매크로가 선언 돼 있어야한다.
		template<typename To, typename From>
		static auto Cast(From* src) -> std::enable_if_t<
			sh::core::Reflection::IsSClass<To>::value && 
			sh::core::Reflection::IsSClass<From>::value, To*>
		{
			if (!src) return nullptr;
			if (src->GetTypeInfo().IsChild(To::GetStaticTypeInfo()))
				return reinterpret_cast<To*>(src);
			return nullptr;
		}
	};
}