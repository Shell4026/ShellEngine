#pragma once
#include "TypeTraits.hpp"
#include "../Util.h"

namespace sh::core::reflection
{
	class STypeInfo;

	template<typename T>
	struct TypeInfoCreateInfo
	{
		const std::string_view name;
		const std::size_t size;
		const std::size_t hash;
		constexpr TypeInfoCreateInfo() :
			name
			(
				TypeTraits::GetTypeName<std::remove_const_t<std::remove_pointer_t<std::remove_reference_t<T>>>>()), 
				size(sizeof(T)), 
				hash(TypeTraits::GetTypeHash<std::remove_const_t<std::remove_pointer_t<std::remove_reference_t<T>>>>()
			)
		{}
	};

	/// @brief 타입에 대한 정보를 담고 있는 구조체.
	/// @brief SCLASS 객체라면 GetSType()으로 더 많은 정보를 얻을 수 있다.
	/// @brief const여부와 포인터 타입에 상관 없이 hash와 name 값은 모두 같게 나온다. 정확한 비교를 위해서는 ==로 비교 해야한다.
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
		const bool isReference;

		template<typename T>
		constexpr TypeInfo(const TypeInfoCreateInfo<T> data) :
			name(data.name), size(data.size), hash(data.hash),
			getTypeFn([]
				{
					if constexpr (reflection::IsSClass<T>::value)
						return &T::GetStaticType; 
					else 
						return nullptr; 
				}()
			),
			containerNestedLevel(reflection::GetContainerNestedCount<T>::value),
			isConst(std::is_const_v<T>), isContainer(IsContainer<T>::value), isSObject(reflection::IsSObject<T>::value),
			isPointer(std::is_pointer_v<T>), isSObjectPointer(std::is_pointer_v<T>&& std::is_convertible_v<T, SObject*>),
			isReference(std::is_reference_v<T>)
		{
		}
		constexpr TypeInfo(const TypeInfo& other) :
			name(other.name), size(other.size), hash(other.hash), 
			getTypeFn(other.getTypeFn),
			containerNestedLevel(other.containerNestedLevel),
			isConst(other.isConst), isContainer(other.isContainer), isSObject(other.isSObject),
			isPointer(other.isPointer), isSObjectPointer(other.isSObjectPointer),
			isReference(other.isReference)
		{}

		constexpr bool operator==(const TypeInfo& other) const
		{
			return 
				isConst == other.isConst && 
				isReference == other.isReference &&
				size == other.size && 
				hash == other.hash;
		}
		constexpr bool operator!=(const TypeInfo& other) const
		{
			return !operator==(other);
		}
		auto GetSType() const -> STypeInfo*
		{
			if (getTypeFn == nullptr)
				return nullptr;
			return &getTypeFn();
		}
	private:
		STypeInfo& (*const getTypeFn)();
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
