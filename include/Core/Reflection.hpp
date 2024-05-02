#pragma once

#include "Export.h"

#include <cstddef>
#include <typeinfo>
#include <type_traits>
#include <utility>
#include <vector>
#include <map>
#include <string>

#define SCLASS(class_name)\
public:\
	using Super = sh::core::reflection::MakeSuper<class_name>::type;\
	using This = class_name;\
public:\
	static auto GetStaticType() -> sh::core::reflection::TypeInfo&\
	{\
		static sh::core::reflection::TypeInfo info{ sh::core::reflection::TypeInfoData<class_name>(#class_name) };\
		return info;\
	}\
	virtual auto GetType() const -> sh::core::reflection::TypeInfo&\
	{\
		return typeInfo;\
	}\
	private:\
		inline static sh::core::reflection::TypeInfo& typeInfo = GetStaticType();

#define PROPERTY(variable_name)\
struct _PropertyFactory_##variable_name\
{\
	_PropertyFactory_##variable_name()\
	{\
		static sh::core::reflection::PropertyInfo\
			<This, \
			decltype(variable_name), \
			decltype(&This::variable_name), \
			&This::variable_name> property_##variable_name{ #variable_name };\
	}\
} _propertyFactory_##variable_name;\

namespace sh::core
{
	class SObject;
}

namespace sh::core::reflection
{
	class TypeInfo;
	class Property;
	
	//기본 HasSuper 구조체
	template<typename T, typename U = void>
	struct HasSuper :
		std::bool_constant<false> {};
	//T가 Super을 가지고 있다면 오버로딩 된다(SFINAE).
	template<typename T>
	struct HasSuper<T, std::void_t<typename T::Super>> :
		std::bool_constant<!std::is_same_v<typename T::Super, void>> {};

	template<typename T, typename CheckThis = void>
	struct HasThis :
		std::bool_constant<false> {};

	template<typename T>
	struct HasThis<T, std::void_t<typename T::This>> :
		std::bool_constant<!std::is_same_v<typename T::This, void>> {};

	template<typename T>
	struct IsSClass : std::bool_constant<HasThis<T>::value> {};

	template<typename T, typename U = void>
	struct MakeSuper {
		using type = U;
	};
	template<typename T>
	struct MakeSuper<T, std::void_t<typename T::This>> {
		using type = typename T::This;
	};

	/// \brief 빠른 다운 캐스팅.
	///
	/// 둘 다 SCLASS매크로가 선언 돼 있어야한다.
	template<typename To, typename From>
	static auto Cast(From* src) -> std::enable_if_t<
		IsSClass<To>::value&&
		IsSClass<From>::value, To*>
	{
		if (!src) return nullptr;
		if (src->GetType().IsChildOf(To::GetStaticType()))
			return reinterpret_cast<To*>(src);
		return nullptr;
	}

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
				super = &(T::Super::GetStaticType());
			}
		}
	};

	class TypeInfo {
	private:
		const char* name;
		const TypeInfo* super;
		const size_t hash;

		std::map<std::string, Property> properties;
		std::vector<Property*> pointers;
	public:
		template<typename T>
		explicit TypeInfo(TypeInfoData<T> data) :
			name(data.name), super(data.super), hash(typeid(T).hash_code()) {}

		SH_CORE_API auto GetName() const -> const char*;
		SH_CORE_API auto GetSuper() const -> const TypeInfo*;
		//other와 자신이 같은 타입인지
		SH_CORE_API bool IsA(const TypeInfo& other) const;
		SH_CORE_API bool operator==(const TypeInfo& other) const;
		//현재 타입이 other의 자식인지
		SH_CORE_API bool IsChildOf(const TypeInfo& other) const;

		SH_CORE_API auto AddProperty(const std::string& name, const Property& prop) -> Property*;
		SH_CORE_API void AddPointerProperty(Property* prop);
		SH_CORE_API auto GetProperty(const std::string& name) -> Property*;
		SH_CORE_API auto GetProperties() const -> const std::map<std::string, Property>&;
		SH_CORE_API auto GetPointerProperties() const -> const std::vector<Property*>&;
	};//TypeInfo

	class PropertyDataBase
	{
	public:
		TypeInfo& type;
	public:
		PropertyDataBase(TypeInfo& type) :
			type(type)
		{
		}
	};

	template<typename T>
	class IPropertyData : public PropertyDataBase
	{
	public:
		IPropertyData<T>(TypeInfo& type) :
			PropertyDataBase(type)
		{
		}
		virtual auto Get(void* sobject) const -> T& = 0;
		virtual void Set(void* sobject, T value) = 0;
	};

	template<typename ThisType, typename T>
	class PropertyData : public IPropertyData<T>
	{
	private:
		T ThisType::* ptr; //맴버변수 포인터
	public:
		PropertyData(T ThisType::* ptr, TypeInfo& type):
			IPropertyData<T>(type),
			ptr(ptr)
		{
		}

		auto Get(void* sobject) const -> T& override 
		{
			return static_cast<ThisType*>(sobject)->*ptr;
		}

		void Set(void* sobject, T value) override
		{
			static_cast<ThisType*>(sobject)->*ptr = value;
		}
	};

	template<typename ThisType, typename T, typename VariablePointer, VariablePointer ptr>
	class PropertyInfo
	{
	private:
		const char* name;
		TypeInfo& owner;
		VariablePointer test;
	public:
		PropertyInfo(const char* name) :
			name(name), owner(ThisType::GetStaticType())
		{
			static PropertyData<ThisType, T> data{ ptr, owner };
			Property* prop = owner.AddProperty(name, Property{ &data });
			if constexpr (std::is_pointer_v<T> && std::is_convertible_v<T, SObject*>)
			{
				if (prop != nullptr)
					owner.AddPointerProperty(prop);
			}
		}
	};

	class Property
	{
	private:
		PropertyDataBase* data;
	public:
		Property(PropertyDataBase* data) :
			data(data)
		{

		}
			
		template<typename T, typename ThisType>
		auto Get(ThisType* sobject) const -> T&
		{
			//if (data->type.IsChild())

			return static_cast<IPropertyData<T>*>(data)->Get(sobject);
		}
		template<typename T, typename ThisType>
		void Set(ThisType* sobject, T value)
		{
			static_cast<IPropertyData<T>*>(data)->Set(sobject, value);
		}
	};
}