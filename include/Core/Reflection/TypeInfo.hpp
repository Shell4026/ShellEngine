#pragma once
#include "TypeTraits.hpp"
#include "../Util.h"

namespace sh::core::reflection
{
	template<typename T>
	struct TypeInfoCreateInfo
	{
		const std::string_view name;
		const std::size_t size;
		const std::size_t hash;
		constexpr TypeInfoCreateInfo() :
			name(TypeTraits::GetTypeName<std::remove_const_t<T>>()), size(sizeof(T)), hash(TypeTraits::GetTypeHash<std::remove_const_t<T>>())
		{}
	};

	/// @brief 타입에 대한 정보를 담고 있는 구조체.
	/// @brief SCLASS 객체는 STypeInfo 클래스를 쓰는 것이 더 좋다.
	struct TypeInfo
	{
		const std::string_view name;
		const std::size_t size;
		const std::size_t hash;
		const uint32_t containerNestedLevel;
		const bool isConst;
		const bool isPointer;
		const bool isContainer;
		const bool isSObject;
		const bool isSObjectPointer;

		template<typename T>
		constexpr TypeInfo(const TypeInfoCreateInfo<T> data) :
			name(data.name), size(data.size), hash(data.hash), containerNestedLevel(reflection::GetContainerNestedCount<T>::value),
			isConst(std::is_const_v<T>), isContainer(IsContainer<T>::value), isSObject(reflection::IsSObject<T>::value),
			isPointer(std::is_pointer_v<T>), isSObjectPointer(std::is_pointer_v<T>&& std::is_convertible_v<T, SObject*>)
		{}
		constexpr TypeInfo(const TypeInfo& other) :
			name(other.name), size(other.size), hash(other.hash), containerNestedLevel(other.containerNestedLevel),
			isConst(other.isConst), isContainer(other.isContainer), isSObject(other.isSObject),
			isPointer(other.isPointer), isSObjectPointer(other.isSObjectPointer)
		{}

		constexpr bool operator==(const TypeInfo& other) const
		{
			return size == other.size && hash == other.hash;
		}
		constexpr bool operator!=(const TypeInfo& other) const
		{
			return !operator==(other);
		}
	};

	/// @brief 해당 타입에 대한 TypeInfo 구조체를 반환 하는 함수
	/// @tparam T 타입
	/// @return TypeInfo 구조체
	template<typename T>
	auto GetType() -> const TypeInfo&
	{
		static TypeInfo typeInfo{ TypeInfoCreateInfo<T>{} };
		return typeInfo;
	}
}//namespace

namespace std
{
	template<>
	struct hash<sh::core::reflection::TypeInfo>
	{
		auto operator()(const sh::core::reflection::TypeInfo& type) const -> std::size_t
		{
			return type.hash;
		}
	};
}//namespace
