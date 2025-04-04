#pragma once
#include "Export.h"
#include "Reflection/TypeTraits.hpp"
#include "Reflection/STypeInfo.hpp"
#include "Reflection/Property.hpp"
#include "Reflection/TypeInfo.hpp"

namespace sh::core::reflection
{
	/// \brief 빠른 다운 캐스팅. 둘 다 SCLASS매크로가 선언 돼 있어야한다.
	template<typename To, typename From>
	static auto Cast(From* src) -> std::enable_if_t<
		IsSClass<To>::value &&
		IsSClass<From>::value, To*>
	{
		if (!src) return nullptr;
		if (src->GetType().IsChildOf(To::GetStaticType()))
			return reinterpret_cast<To*>(src);
		return nullptr;
	}
}//namespace