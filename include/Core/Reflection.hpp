#pragma once

#include "Export.h"

#include <cstddef>
#include <typeinfo>
#include <type_traits>
#include <utility>
#include <vector>
#include <string>
#include <string_view>
#include <cassert>
#include <memory>
#include <cstring>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>

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

#define PROPERTY(variable_name, ...)\
struct _PropertyFactory_##variable_name\
{\
	_PropertyFactory_##variable_name()\
	{\
		static sh::core::reflection::PropertyInfo\
			<This, \
			decltype(variable_name), \
			decltype(&This::variable_name), \
			&This::variable_name> property_##variable_name{ #variable_name, std::string_view{#__VA_ARGS__} };\
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

	template<typename T>
	struct IsSObject : std::bool_constant<std::is_base_of_v<SObject, T>> {};

	template<typename T, typename U = void>
	struct MakeSuper {
		using type = U;
	};
	template<typename T>
	struct MakeSuper<T, std::void_t<typename T::This>> {
		using type = typename T::This;
	};

	template<typename T, typename Check = void>
	struct IsContainer : std::bool_constant<false> {};
	template<typename T>
	struct IsContainer<T, std::void_t<decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())>> : std::bool_constant<true> {};

	template<typename T, typename Check = void>
	struct IsPair : std::bool_constant<false> {};
	template<typename T>
	struct IsPair<T, std::void_t<typename T::first_type, typename T::second_type>> : std::bool_constant<true> {};

	template<typename T>
	struct IsVector : std::bool_constant<false>{};
	template<typename T>
	struct IsVector<std::vector<T>> : std::bool_constant<true> {};

	template<typename T, typename U = void>
	struct IsMap : std::bool_constant<false> {};
	template<typename T, typename U>
	struct IsMap<std::map<T, U>, void> : std::bool_constant<true> {};

	// 컨테이너 중첩 수 구하기
	template<typename T>
	struct GetContainerNestedCount : std::integral_constant<int, 0> {};
	template<typename T>
	struct GetContainerNestedCount<std::vector<T>> : std::integral_constant<int, GetContainerNestedCount<T>::value + 1> {};
	template<typename T, typename U, typename _Pr, typename _Alloc>
	struct GetContainerNestedCount<std::map<T, U, _Pr, _Alloc>> : std::integral_constant<int, GetContainerNestedCount<U>::value + 1> {};
	template<typename T, typename U, typename _Hasher, typename _Keyeq, typename _Alloc>
	struct GetContainerNestedCount<std::unordered_map<T, U, _Hasher, _Keyeq, _Alloc>> : std::integral_constant<int, GetContainerNestedCount<U>::value + 1> {};
	template<typename T, typename _Pr, typename _Alloc>
	struct GetContainerNestedCount<std::set<T, _Pr, _Alloc>> : std::integral_constant<int, GetContainerNestedCount<T>::value + 1> {};
	template<typename T, typename _Hasher, typename _Keyeq, typename _Alloc>
	struct GetContainerNestedCount<std::unordered_set<T, _Hasher, _Keyeq, _Alloc>> : std::integral_constant<int, GetContainerNestedCount<T>::value + 1> {};

	// 중첩된 컨테이너의 마지막 자료형 구하기
	template<typename T>
	struct GetContainerLastType
	{
		using type = T;
	};
	template<typename T>
	struct GetContainerLastType<std::vector<T>>
	{
		using type = typename GetContainerLastType<T>::type;
	};
	template<typename T, typename U, typename _Pr, typename _Alloc>
	struct GetContainerLastType<std::map<T, U, _Pr, _Alloc>>
	{
		using type = typename GetContainerLastType<U>::type;
	};
	template<typename T, typename U, typename _Hasher, typename _Keyeq, typename _Alloc>
	struct GetContainerLastType<std::unordered_map<T, U, _Hasher, _Keyeq, _Alloc>>
	{
		using type = typename GetContainerLastType<U>::type;
	};
	template<typename T, typename _Pr, typename _Alloc>
	struct GetContainerLastType<std::set<T, _Pr, _Alloc>>
	{
		using type = typename GetContainerLastType<T>::type;
	};
	template<typename T, typename _Hasher, typename _Keyeq, typename _Alloc>
	struct GetContainerLastType<std::unordered_set<T, _Hasher, _Keyeq, _Alloc>>
	{
		using type = typename GetContainerLastType<T>::type;
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
	constexpr auto GetTypeName() -> std::string_view
	{

#ifdef __clang__
		std::string_view prefix = "auto sh::core::reflection::GetTypeName() [T = ";
		std::string_view suffix = "]";
		std::string_view str = __PRETTY_FUNCTION__;
#elif __GNUC__
		std::string_view prefix = "constexpr auto sh::core::reflection::GetTypeName() [with T = ";
		std::string_view suffix = "]";
		std::string_view str = __PRETTY_FUNCTION__;
#elif _MSC_VER
		std::string_view prefix = "auto __cdecl sh::core::reflection::GetTypeName<";
		std::string_view suffix = ">(void)";
		std::string_view str = __FUNCSIG__;
#endif
		str.remove_prefix(prefix.size());
		str.remove_suffix(suffix.size());

		return str;
	}

	template<typename T>
	struct TypeInfoData
	{
		std::string_view name;
		const TypeInfo* super;
		using Type = T;

		TypeInfoData(std::string_view name) :
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
		std::string_view name;
		const TypeInfo* super;
		const size_t hash;

		std::map<std::string, Property> properties;
		std::vector<Property*> pointers;
		std::vector<Property*> containers;
	public:
		const bool isPointer;
		const size_t size;
	public:
		template<typename T>
		explicit TypeInfo(TypeInfoData<T> data) :
			name(data.name), super(data.super), hash(typeid(T).hash_code()), 
			isPointer(std::is_pointer_v<T>), size(sizeof(T))
		{
		}

		SH_CORE_API auto GetName() const -> std::string_view;
		SH_CORE_API auto GetSuper() const -> const TypeInfo*;
		//other와 자신이 같은 타입인지
		SH_CORE_API bool IsA(const TypeInfo& other) const;
		SH_CORE_API bool operator==(const TypeInfo& other) const;
		//현재 타입이 other의 자식인지
		SH_CORE_API bool IsChildOf(const TypeInfo& other) const;

		SH_CORE_API auto AddProperty(const std::string& name, const Property& prop) -> Property*;
		SH_CORE_API auto GetProperty(const std::string& name) -> Property*;
		SH_CORE_API auto GetProperties() const -> const std::map<std::string, Property>&;

		SH_CORE_API void AddSObjectPtrProperty(Property* prop);
		SH_CORE_API auto GetSObjectPtrProperties() const -> const std::vector<Property*>&;

		SH_CORE_API void AddSObjectContainerProperty(Property* prop);
		SH_CORE_API auto GetSObjectContainerProperties() const -> const std::vector<Property*>&;
	};//TypeInfo

	class IPropertyIteratorBase;
	template<typename T>
	class IPropertyIterator;

	//추상화된 프로퍼티 반복자
	class PropertyIterator
	{
	private:
		std::unique_ptr<IPropertyIteratorBase> iteratorData;
	public:
		SH_CORE_API PropertyIterator();
		SH_CORE_API PropertyIterator(std::unique_ptr<IPropertyIteratorBase>&& iteratorData);
		SH_CORE_API PropertyIterator(PropertyIterator&& other) noexcept;

		SH_CORE_API auto operator==(const PropertyIterator& other) -> bool;
		SH_CORE_API auto operator!=(const PropertyIterator& other) -> bool;
		SH_CORE_API auto operator++()->PropertyIterator&;
		SH_CORE_API auto operator=(PropertyIterator&& other) noexcept -> PropertyIterator&;

		SH_CORE_API auto GetTypeName() const->std::string_view;

		template<typename T>
		auto Get() -> T*;
		template<typename T>
		auto GetPairSecond() -> T*;

		/// @brief 자식 컨테이너의 반복자를 가져오는 함수
		/// @return 프로퍼티 반복자
		SH_CORE_API auto GetNestedBegin() -> PropertyIterator;
		SH_CORE_API auto GetNestedEnd() -> PropertyIterator;
		SH_CORE_API auto IsPair() const -> bool;
	};

	//자료형과 컨테이너를 숨긴 프로퍼티 반복자 인터페이스//
	class IPropertyIteratorBase
	{
	public:
		std::string_view typeName;
	public:
		virtual ~IPropertyIteratorBase() = default;
		virtual void operator++() = 0;
		virtual auto operator==(const IPropertyIteratorBase& other) -> bool = 0;
		virtual auto operator!=(const IPropertyIteratorBase& other) -> bool = 0;

		virtual auto GetNestedBegin() -> PropertyIterator = 0;
		virtual auto GetNestedEnd() -> PropertyIterator = 0;

		virtual auto IsPair() const -> bool = 0;
		virtual auto GetPairSecond() const -> void* = 0;
	};

	//컨테이너를 숨긴 프로퍼티 반복자 인터페이스//
	template<typename T>
	class IPropertyIterator : public IPropertyIteratorBase
	{
	public:
		virtual void Begin() = 0;
		virtual void End() = 0;
		virtual auto Get() -> T& = 0;
	};

	//실질적인 프로퍼티 반복자 데이터//
	template<typename TContainer, typename T = typename TContainer::value_type>
	class PropertyIteratorData : public IPropertyIterator<T>
	{
	private:
		TContainer* container;
		typename TContainer::iterator it;
	public:
		PropertyIteratorData(TContainer* container) :
			container(container)
		{
			IPropertyIteratorBase::typeName = GetTypeName<T>();
		}

		void Begin() override
		{
			assert(container);
			it = container->begin();
		}

		void End() override
		{
			assert(container);
			it = container->end();
		}

		auto Get() -> T& override
		{
			return *it;
		}

		auto GetPairSecond() const -> void*
		{
			if constexpr(!sh::core::reflection::IsPair<T>::value)
				return nullptr;
			else
				return &it->second;
		}

		auto GetNestedBegin() -> PropertyIterator override
		{
			if constexpr (GetContainerNestedCount<TContainer>::value > 1)
			{
				if constexpr (sh::core::reflection::IsPair<T>::value)
				{
					auto data = std::make_unique<PropertyIteratorData<typename T::second_type>>(&(it->second));
					data->Begin();
					return PropertyIterator{ std::move(data) };
				}
				else
				{
					auto data = std::make_unique<PropertyIteratorData<T>>(&*it);
					data->Begin();
					return PropertyIterator{ std::move(data) };
				}
			}
			return PropertyIterator{};
		}

		auto GetNestedEnd() -> PropertyIterator override
		{
			if constexpr (GetContainerNestedCount<TContainer>::value > 1)
			{
				if constexpr (sh::core::reflection::IsPair<T>::value)
				{
					auto data = std::make_unique<PropertyIteratorData<typename T::second_type>>(&(it->second));
					data->End();
					return PropertyIterator{ std::move(data) };
				}
				else
				{
					auto data = std::make_unique<PropertyIteratorData<T>>(&*it);
					data->End();
					return PropertyIterator{ std::move(data) };
				}
			}
			return PropertyIterator{};
		}

		auto IsPair() const -> bool override
		{
			return sh::core::reflection::IsPair<T>::value;
		}

		auto operator==(const IPropertyIteratorBase& other) -> bool override
		{
			return static_cast<const PropertyIteratorData<TContainer, T>*>(&other)->it == it;
		}

		auto operator!=(const IPropertyIteratorBase& other) -> bool override
		{
			return static_cast<const PropertyIteratorData<TContainer, T>*>(&other)->it != it;
		}

		void operator++() override
		{
			++it;
		}
	};

	class PropertyDataBase
	{
	protected:
		std::string_view typeName;
	public:
		TypeInfo& ownerType;

		bool isConst = false;
		bool bVisible = true;
	public:
		PropertyDataBase(TypeInfo& ownerType) :
			ownerType(ownerType), typeName("")
		{
		}

		SH_CORE_API auto GetTypeName() const -> std::string_view;
		virtual auto Begin(void* sobject) const->PropertyIterator = 0;
		virtual auto End(void* sobject) const->PropertyIterator = 0;
	};

	template<typename T>
	auto PropertyIterator::Get() -> T*
	{
		//컨테이너 클래스가 아닌경우를 뜻한다.
		if (iteratorData.get() == nullptr)
			return nullptr;

		IPropertyIterator<T>* it = static_cast<IPropertyIterator<T>*>(iteratorData.get());
		return &it->Get();
	}

	template<typename T>
	auto PropertyIterator::GetPairSecond() -> T*
	{
		return reinterpret_cast<T*>(iteratorData->GetPairSecond());
	}

	template<typename T>
	class IPropertyData : public PropertyDataBase
	{
	public:
		IPropertyData<T>(TypeInfo& ownerType) :
			PropertyDataBase(ownerType)
		{
		}
		virtual auto Get(void* sobject) const -> T& = 0;
	};
	
	//클래스의 맴버 변수마다 static영역에 하나씩 존재하는 클래스
	template<typename ThisType, typename T, typename VariablePointer, VariablePointer ptr>
	class PropertyData : public IPropertyData<T>
	{
	public:
		PropertyData(TypeInfo& ownerType) :
			IPropertyData<T>(ownerType)
		{
			PropertyDataBase::typeName = sh::core::reflection::GetTypeName<T>();
		}

		auto Get(void* sobject) const -> T & override
		{
			if constexpr (std::is_member_pointer_v<VariablePointer>)
				return static_cast<ThisType*>(sobject)->*ptr;
			else
				return *ptr;
		}

		auto Begin(void* sobject) const -> PropertyIterator override
		{
			if constexpr (IsContainer<T>())
			{
				if constexpr (std::is_member_pointer_v<VariablePointer>)
				{
					T& container = static_cast<ThisType*>(sobject)->*ptr;
					std::unique_ptr<PropertyIteratorData<T>> data = std::make_unique<PropertyIteratorData<T>>(&container);
					data->Begin();
					return PropertyIterator{ std::move(data) };
				}
				else
				{
					T& container = *ptr;
					std::unique_ptr<PropertyIteratorData<T>> data = std::make_unique<PropertyIteratorData<T>>(&container);
					data->Begin();
					return PropertyIterator{ std::move(data) };
				}
			}
			return PropertyIterator{};
		}

		auto End(void* sobject) const -> PropertyIterator override
		{
			if constexpr (IsContainer<T>())
			{
				if constexpr (std::is_member_pointer_v<VariablePointer>)
				{
					T& container = static_cast<ThisType*>(sobject)->*ptr;
					std::unique_ptr<PropertyIteratorData<T>> data = std::make_unique<PropertyIteratorData<T>>(&container);
					data->End();
					return PropertyIterator{ std::move(data) };
				}
				else
				{
					T& container = *ptr;
					std::unique_ptr<PropertyIteratorData<T>> data = std::make_unique<PropertyIteratorData<T>>(&container);
					data->End();
					return PropertyIterator{ std::move(data) };
				}
			}
			return PropertyIterator{};
		}
	};

	//프로퍼티를 만드는데 필요한 정보를 담고 있는 클래스
	template<typename ThisType, typename T, typename VariablePointer, VariablePointer ptr>
	class PropertyInfo
	{
	private:
		TypeInfo& owner;

		const char* name;

		struct Option
		{
			bool isConst = false;
			bool bVisible = true;
		};
	private:
		constexpr static auto ParseOption(std::string_view option) -> Option
		{
			Option retOption{};
			size_t start = 0;
			for (size_t i = 0; i <= option.size(); ++i) {
				if (i == option.size() || option[i] == ' ' || option[i] == ',') {
					std::string_view token = option.substr(start, i - start);
					if (token == "const")
						retOption.isConst = true;
					else if (token == "invisible")
						retOption.bVisible = false;
					start = i + 1;
				}
			}
			
			return retOption;
		}
	public:
		PropertyInfo(const char* name, std::string_view option) :
			name(name), owner(ThisType::GetStaticType())
		{
			Option options = ParseOption(option);

			static PropertyData<ThisType, T, VariablePointer, ptr> data{ owner };
			data.isConst = options.isConst;
			data.bVisible = options.bVisible;
			

			if constexpr (std::is_convertible_v<T, SObject*>)
			{
				Property* prop = owner.AddProperty(name, Property{ &data, name });
				owner.AddSObjectPtrProperty(prop);
			}
			else if (IsContainer<T>())
			{
				using type = typename GetContainerLastType<T>::type;
				if constexpr (std::is_convertible_v<type, SObject*>)
				{
					Property* prop = owner.AddProperty(name, Property{ &data, name, true, GetContainerNestedCount<T>::value });
					owner.AddSObjectContainerProperty(prop);
				}
				else
					Property* prop = owner.AddProperty(name, Property{ &data, name });
			}
			else
			{
				Property* prop = owner.AddProperty(name, Property{ &data, name });
			}
		}
	};

	//실제 프로퍼티 클래스
	class Property
	{
	private:
		PropertyDataBase* data;

		const char* name;
	public:
		const bool isContainer;
		const int containerNestedLevel;
		const bool isConst;
		const bool bVisible;
	public:
		SH_CORE_API Property(PropertyDataBase* data, const char* name, bool isContainer = false, uint32_t containerNestedLevel = 0);
			
		/// @brief 프로퍼티가 가지고 있는 값을 반환하는 함수.
		/// 
		/// 주의: 타입 검사를 하지 않음.
		/// @tparam T 반환 타입
		/// @tparam ThisType SObject의 타입
		/// @param sobject 해당 프로퍼티를 가지고 있는 SObject객체
		/// @return 실제 값
		template<typename T, typename ThisType>
		auto Get(ThisType* sobject) const -> T*
		{
			return &static_cast<IPropertyData<T>*>(data)->Get(sobject);
		}
		SH_CORE_API auto Begin(SObject* SObject) -> PropertyIterator;
		SH_CORE_API auto End(SObject* SObject) -> PropertyIterator;
		SH_CORE_API auto GetName() const -> const char*;
		SH_CORE_API auto GetTypeName() const -> std::string_view;
	};
}