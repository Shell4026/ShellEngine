#pragma once

#include "Export.h"

#include <cstddef>
#include <typeinfo>
#include <type_traits>
#include <utility>

#define SCLASS(class_name)\
public:\
	using Super = sh::core::Reflection::MakeSuper<class_name>::type;\
	using This = class_name;\
public:\
	static auto GetStaticTypeInfo() -> const sh::core::Reflection::TypeInfo&\
	{\
		static sh::core::Reflection::TypeInfo info{ sh::core::Reflection::TypeInfoData<class_name>(#class_name) };\
		return info;\
	}\
	virtual auto GetTypeInfo() const -> const sh::core::Reflection::TypeInfo&\
	{\
		return typeInfo;\
	}\
private:\
	inline static const sh::core::Reflection::TypeInfo& typeInfo = GetStaticTypeInfo();

namespace sh::core
{
	class SH_CORE_API Reflection
	{
	public:
		class TypeInfo;

		template<typename T>
		struct TypeInfoData
		{
			const char* name;
			const TypeInfo* super;

			TypeInfoData(const char* name) :
				name(name), super(nullptr)
			{
				if constexpr (HasSuper<T>())
				{
					super = &(T::Super::GetStaticTypeInfo());
				}
			}
		};

		class SH_CORE_API TypeInfo {
		private:
			const char* name;
			const TypeInfo* super;
			const size_t hash;
		public:
			template<typename T>
			explicit TypeInfo(TypeInfoData<T> data) :
				name(data.name), super(data.super), hash(typeid(T).hash_code()) {}

			auto GetName() const -> const char*;
			auto GetSuper() const -> const TypeInfo*;
			bool IsA(const TypeInfo& other) const;
			bool IsChild(const TypeInfo& other) const;
		};
	public:
		template<typename T, typename U = void>
		struct MakeSuper {
			using type = U;
		};
		template<typename T>
		struct MakeSuper<T, std::void_t<typename T::This>> {
			using type = typename T::This;
		};

		//기본 HasSuper 구조체
		template<typename T, typename U = void>
		struct HasSuper :
			std::integral_constant<bool, false> {};
		//T가 Super을 가지고 있다면 오버로딩 된다(SFINAE). 없으면 기본HasSuper
		template<typename T>
		struct HasSuper<T, std::void_t<typename T::Super>> :
			std::integral_constant<bool, !std::is_same_v<typename T::Super, void>>{};
	};

}