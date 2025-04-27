#pragma once
#include "TypeTraits.hpp"
#include "TypeInfo.hpp"
#include "../Name.h"

#include <string_view>
#include <initializer_list>
#include <type_traits>
#include <optional>
#include <cassert>

#define PROPERTY(variable_name, ...)\
struct _PropertyFactory_##variable_name\
{\
	_PropertyFactory_##variable_name()\
	{\
		sh::core::reflection::PropertyCreateInfo\
			<This, \
			decltype(variable_name), \
			decltype(&This::variable_name), \
			&This::variable_name> property_##variable_name{ #variable_name, {__VA_ARGS__} };\
		GetStaticType().AddProperty(sh::core::reflection::Property{ property_##variable_name });\
	}\
};\
inline static _PropertyFactory_##variable_name _propertyFactory_##variable_name{};

namespace sh::core
{
	class SObject;
	struct PropertyOption
	{
		static constexpr const char* invisible = "invisible";
		static constexpr const char* constant = "const";
		static constexpr const char* noSave = "noSave";
	};
}//namespace

namespace sh::core::reflection
{
	class IPropertyIteratorBase;
	template<typename T>
	class IPropertyIterator;
	template<typename TContainer, typename T = typename TContainer::value_type>
	class PropertyIteratorData;

	//추상화된 프로퍼티 반복자
	class PropertyIterator
	{
	private:
		alignas(alignof(std::max_align_t)) uint8_t iteratorBuffer[48]; // 힙 할당을 최소화 하기 위해 도입, 메모리 정렬도 맞춰야 성능 보장
		IPropertyIteratorBase* iteratorDataPtr = nullptr;
	public:
		SH_CORE_API PropertyIterator();
		template<typename TContainer, typename T = typename TContainer::value_type>
		PropertyIterator(const PropertyIteratorData<TContainer, T>& data)
		{
			static_assert(sizeof(PropertyIteratorData<TContainer, T>) <= sizeof(iteratorBuffer), "Iterator size exceeds buffer");
			iteratorDataPtr = new (iteratorBuffer) PropertyIteratorData<TContainer, T>{data};
		}
		SH_CORE_API PropertyIterator(const PropertyIterator& other);
		SH_CORE_API ~PropertyIterator();

		SH_CORE_API auto operator=(const PropertyIterator& other) -> PropertyIterator&;
		SH_CORE_API auto operator==(const PropertyIterator& other) -> bool;
		SH_CORE_API auto operator!=(const PropertyIterator& other) -> bool;
		SH_CORE_API auto operator++() -> PropertyIterator&;

		/// @brief 원소의 타입을 반환 하는 함수
		/// @return 타입 객체
		SH_CORE_API auto GetType() const -> const TypeInfo&;
		/// @brief Pair타입 원소의 타입을 반환 하는 함수
		/// @return Pair가 아니라면 {}를 반환, 맞다면 Pair의 first와 second 타입을 반환
		SH_CORE_API auto GetPairType() const -> std::optional<std::pair<TypeInfo, TypeInfo>>;
		template<typename T>
		auto Get() -> T*
		{
			//컨테이너 클래스가 아닌경우를 뜻한다.
			if (iteratorDataPtr == nullptr)
				return nullptr;

			IPropertyIterator<T>* it = static_cast<IPropertyIterator<T>*>(iteratorDataPtr);
			return &it->Get();
		}

		/// @brief 값이 std::pair일 경우 페어의 첫번째 값을 반환 하는 함수
		/// @brief pair가 아닐 시 nullptr를 반환한다.
		/// @tparam T 타입
		/// @return pair->first
		template<typename T>
		auto GetPairFirst() -> T*
		{
			return reinterpret_cast<T*>(iteratorDataPtr->GetPairFirst());
		}
		/// @brief 값이 std::pair일 경우 페어의 두번째 값을 반환 하는 함수
		/// @brief pair가 아닐 시 nullptr를 반환한다.
		/// @tparam T 타입
		/// @return pair->second
		template<typename T>
		auto GetPairSecond() -> T*
		{
			return reinterpret_cast<T*>(iteratorDataPtr->GetPairSecond());
		}

		/// @brief 자식 컨테이너의 반복자를 가져오는 함수
		/// @return 프로퍼티 반복자
		SH_CORE_API auto GetNestedBegin() -> PropertyIterator;
		SH_CORE_API auto GetNestedEnd() -> PropertyIterator;

		SH_CORE_API auto IsPair() const -> bool;
		/// @brief 원소가 const 변수인지 반환하는 함수.
		/// @return 맞으면 true, 아니면 false
		SH_CORE_API auto IsConst() const -> bool;

		/// @brief 컨테이너에서 해당 반복자 위치의 원소를 지우는 함수.
		SH_CORE_API void Erase();
	};

	/// @brief 자료형과 컨테이너를 숨긴 프로퍼티 반복자 인터페이스
	class IPropertyIteratorBase
	{
	public:
		virtual ~IPropertyIteratorBase() = default;
		virtual void operator++() = 0;
		virtual auto operator==(const IPropertyIteratorBase& other) -> bool = 0;
		virtual auto operator!=(const IPropertyIteratorBase& other) -> bool = 0;

		virtual auto GetNestedBegin() -> PropertyIterator = 0;
		virtual auto GetNestedEnd() -> PropertyIterator = 0;

		virtual auto IsPair() const -> bool = 0;
		virtual auto IsConst() const -> bool = 0;

		virtual auto GetPairFirst() const -> void* = 0;
		virtual auto GetPairSecond() const -> void* = 0;

		virtual void Erase() = 0;

		virtual auto GetType() const -> const TypeInfo& = 0;
		virtual auto GetPairType() const->std::optional<std::pair<TypeInfo, TypeInfo>> = 0;

		virtual auto Clone(void* buffer) const -> IPropertyIteratorBase* = 0;
	};

	/// @brief 컨테이너 타입을 숨긴 프로퍼티 반복자 인터페이스
	/// @tparam T 컨테이너가 담고 있는 값의 타입
	template<typename T>
	class IPropertyIterator : public IPropertyIteratorBase
	{
	public:
		auto GetType() const -> const TypeInfo& override
		{
			return reflection::GetType<T>();
		}
		auto GetPairType() const -> std::optional<std::pair<TypeInfo, TypeInfo>> override
		{
			if constexpr (reflection::IsPair<T>::value)
				return std::make_pair(reflection::GetType<typename T::first_type>(), reflection::GetType<typename T::second_type>());
			else
				return {};
		}
		virtual void Begin() = 0;
		virtual void End() = 0;
		virtual auto Get() -> T& = 0;
	};

	/// @brief 실질적인 프로퍼티 반복자 데이터
	/// @tparam TContainer 컨테이너 타입
	/// @tparam T 컨테이너가 담고 있는 값의 타입
	template<typename TContainer, typename T>
	class PropertyIteratorData : public IPropertyIterator<T>
	{
	private:
		using IteratorType = typename std::iterator_traits<typename TContainer::iterator>::reference;

		typename TContainer::iterator it;
		TContainer* container = nullptr;
	public:
		PropertyIteratorData(TContainer* container, typename TContainer::iterator initIterator) :
			container(container), it(initIterator)
		{
		}

		void Begin() override
		{
			it = container->begin();
		}
		void End() override
		{
			it = container->end();
		}

		auto Get() -> T& override
		{
			return const_cast<T&>(*it);
		}
		auto GetPairFirst() const -> void* override
		{
			if constexpr (!sh::core::reflection::IsPair<T>::value)
				return nullptr;
			else
				return const_cast<void*>(reinterpret_cast<const void*>(&it->first)); // ptr*const*일 수도 있으니 const체크 잘 해야함
		}
		auto GetPairSecond() const -> void* override
		{
			if constexpr (!sh::core::reflection::IsPair<T>::value)
				return nullptr;
			else
				return &it->second;
		}

		/// @brief 내부 중첩된 컨테이너의 시작 반복자를 반환하는 함수
		/// @return 반복자
		auto GetNestedBegin() -> PropertyIterator override
		{
			if constexpr (GetContainerNestedCount<TContainer>::value > 1)
			{
				if constexpr (sh::core::reflection::IsPair<T>::value)
				{
					using NestedType = typename T::second_type;
					return PropertyIterator{ PropertyIteratorData<NestedType>{ &it->second, it->second.begin() } };
				}
				else
				{
					return PropertyIterator{ PropertyIteratorData<T>{ &(*it), it->begin() } };
				}
			}
			return PropertyIterator{};
		}
		/// @brief 내부 중첩된 컨테이너의 끝 반복자를 반환하는 함수
		/// @return 반복자
		auto GetNestedEnd() -> PropertyIterator override
		{
			if constexpr (GetContainerNestedCount<TContainer>::value > 1)
			{
				if constexpr (sh::core::reflection::IsPair<T>::value)
				{
					using NestedType = typename T::second_type;
					return PropertyIterator{ PropertyIteratorData<NestedType>{ &it->second, it->second.end() } };
				}
				else
				{
					return PropertyIterator{ PropertyIteratorData<T>{ &(*it), it->end() } };
				}
			}
			return PropertyIterator{};
		}

		/// @brief 해당 타입이 std::pair인지
		/// @return 맞다면 true 아니면 false
		auto IsPair() const -> bool override
		{
			return sh::core::reflection::IsPair<T>::value;
		}
		/// @brief 해당 타입이 const인지
		/// @return 맞다면 true 아니면 false
		auto IsConst() const -> bool override
		{
			return std::is_const<std::remove_reference_t<IteratorType>>::value;
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
			if (it != container->end())
				++it;
		}

		/// @brief erase가 되는 컨테이너라면 해당 값을 컨테이너에서 erase한다.
		/// @brief 아니라면 nullptr로 만든다.
		void Erase() override
		{
			if constexpr (HasErase<TContainer>::value)
				it = container->erase(it);
			else
			{
				if constexpr (std::is_pointer_v<T> && !std::is_const_v<std::remove_reference_t<IteratorType>>)
					*it = nullptr;
			}
		}

		auto Clone(void* buffer) const -> IPropertyIteratorBase* override
		{
			new (buffer) PropertyIteratorData<TContainer, T>{ *this };
			return reinterpret_cast<PropertyIteratorData<TContainer, T>*>(buffer);
		}
	};

	/// @brief 타입을 숨긴 프로퍼티 추상 클래스
	class PropertyDataBase
	{
	public:
		virtual auto GetType() const -> const TypeInfo& = 0;
		virtual auto Begin(void* sobject) const -> PropertyIterator = 0;
		virtual auto End(void* sobject) const -> PropertyIterator = 0;
	};
	/// @brief 타입을 가진 프로퍼티 추상 클래스
	template<typename T>
	class IPropertyData : public PropertyDataBase
	{
	public:
		auto GetType() const -> const TypeInfo& override
		{
			return reflection::GetType<T>();
		}
		virtual auto Get(void* sobject) const -> T& = 0;
		virtual auto Get(const void* sobject) const -> const T& = 0;
	};

	/// @brief 클래스의 맴버 변수마다 static영역에 하나씩 존재하는 클래스
	template<typename ThisType, typename T, typename VariablePointer, VariablePointer ptr>
	class PropertyData : public IPropertyData<T>
	{
	public:
		auto Get(void* sobject) const -> T& override
		{
			if constexpr (std::is_member_pointer_v<VariablePointer>)
				return static_cast<ThisType*>(sobject)->*ptr;
			else
				return *ptr;
		}
		auto Get(const void* sobject) const -> const T& override
		{
			if constexpr (std::is_member_pointer_v<VariablePointer>)
				return static_cast<const ThisType*>(sobject)->*ptr;
			else
				return *ptr;
		}

		/// @brief 해당 프로퍼티가 컨테이너라면 시작 반복자를 반환한다.
		/// @param sobject 해당 프로퍼티를 가지고 있는 클래스 포인터
		/// @return 컨테이너라면 유효한 반복자를, 아니라면 빈 반복자를 반환한다.
		auto Begin(void* sobject) const -> PropertyIterator override
		{
			if constexpr (IsContainer<T>())
			{
				if constexpr (std::is_member_pointer_v<VariablePointer>)
				{
					T& container = static_cast<ThisType*>(sobject)->*ptr;
					return PropertyIterator{ PropertyIteratorData<T>{ &container, container.begin() } };
				}
				else
				{
					T& container = *ptr;
					return PropertyIterator{ PropertyIteratorData<T>{ &container, container.begin() } };
				}
			}
			else
				return PropertyIterator{};
		}
		/// @brief 해당 프로퍼티가 컨테이너라면 시작 반복자를 반환한다.
		/// @param sobject 해당 프로퍼티를 가지고 있는 클래스 포인터
		/// @return 컨테이너라면 유효한 반복자를, 아니라면 빈 반복자를 반환한다.
		auto End(void* sobject) const -> PropertyIterator override
		{
			if constexpr (IsContainer<T>())
			{
				if constexpr (std::is_member_pointer_v<VariablePointer>)
				{
					T& container = static_cast<ThisType*>(sobject)->*ptr;
					return PropertyIterator{ PropertyIteratorData<T>{ &container, container.end() } };
				}
				else
				{
					T& container = *ptr;
					return PropertyIterator{ PropertyIteratorData<T>{ &container, container.end() } };
				}
			}
			else
				return PropertyIterator{};
		}
	};
	/// @brief 프로퍼티를 만드는데 필요한 정보를 담고 있는 클래스
	/// @tparam ThisType 프로퍼티를 가지고 있는 객체의 타입
	/// @tparam T 프로퍼티의 타입
	/// @tparam VariablePointer 맴버 변수 포인터의 타입
	/// @tparam ptr 맴버 변수 포인터
	template<typename ThisType, typename T, typename VariablePointer, VariablePointer ptr>
	struct PropertyCreateInfo
	{
		struct Option
		{
			bool bConst = false;
			bool bVisible = true;
			bool bNoSave = false;
		} option;

		const std::string_view name;

		static auto ParseOption(const std::initializer_list<std::string_view>& option) -> Option
		{
			Option retOption{};
			for (auto& option : option)
			{
				if (option == "const")
					retOption.bConst = true;
				else if (option == "invisible")
					retOption.bVisible = false;
				else if (option == "noSave")
					retOption.bNoSave = true;
			}
			return retOption;
		}
		PropertyCreateInfo(std::string_view name, const std::initializer_list<std::string_view>& option) :
			name(name), option(ParseOption(option))
		{
		}
	};

	/// @brief 프로퍼티 클래스
	class Property
	{
	private:
		PropertyDataBase* data;

		core::Name name;

		const uint32_t containerNestedLevel;
	public:
		const TypeInfo& type;
		const std::string_view pureTypeName;
		const bool bConstProperty;
		const bool bVisibleProperty;
		const bool bNoSaveProperty;
		const bool isConst;
		const bool isPointer;
		const bool isContainer;
		const bool isSObject;
		const bool isSObjectPointer;
		const bool isSObjectPointerContainer;
	public:
		template<typename ThisType, typename T, typename VariablePointer, VariablePointer ptr>
		Property(const PropertyCreateInfo<ThisType, T, VariablePointer, ptr>& createInfo) :
			type(GetType<T>()),
			pureTypeName(GetTypeName<std::remove_const_t<std::remove_pointer_t<std::remove_reference_t<T>>>>()),
			name(createInfo.name),
			containerNestedLevel(GetContainerNestedCount<T>::value),
			bConstProperty(createInfo.option.bConst),
			bVisibleProperty(createInfo.option.bVisible),
			bNoSaveProperty(createInfo.option.bNoSave),
			isConst(std::is_const_v<T>),
			isPointer(std::is_pointer_v<T>),
			isContainer(IsContainer<T>::value),
			isSObject(IsSObject<T>::value),
			isSObjectPointer(std::is_convertible_v<T, const SObject*>),
			isSObjectPointerContainer(reflection::IsContainer<T>::value && std::is_convertible_v<typename reflection::GetContainerLastType<T>::type, const SObject*>)
		{
			// 메모) 템플릿 인자로 인해 클래스 맴버 변수 별로 메모리 상에 하나만 존재하게 된다.
			static PropertyData<ThisType, T, VariablePointer, ptr> data{};
			this->data = &data;
		}
		SH_CORE_API Property(const Property& other) noexcept;

		/// @brief 프로퍼티가 가지고 있는 값을 반환하는 함수.
		/// @brief 주의: 타입 검사를 하지 않음.
		/// @tparam T 반환 타입
		/// @tparam ThisType SObject의 타입
		/// @param sobject 해당 프로퍼티를 가지고 있는 SObject객체
		/// @return 실제 값
		template<typename T, typename ThisType>
		auto Get(ThisType* sobject) const -> T*
		{
			assert(isConst ? std::is_const_v<T> : true);
			return &static_cast<IPropertyData<T>*>(data)->Get(sobject);
		}
		template<typename T, typename ThisType>
		auto Get(const ThisType* sobject) const -> const T*
		{
			return &static_cast<IPropertyData<T>*>(data)->Get(sobject);
		}
		template<typename T, typename ThisType>
		auto GetSafe(ThisType* sobject) const -> T*
		{
			if (data->GetType() != reflection::GetType<T>())
				return nullptr;
 			if (isConst ? !std::is_const_v<T> : false)
				return nullptr;
			return Get<T, ThisType>(sobject);
		}
		template<typename T, typename ThisType>
		auto GetSafe(const ThisType* sobject) const -> const T*
		{
			if (data->GetType() != reflection::GetType<T>())
				return nullptr;
			return Get<T, ThisType>(sobject);
		}
		/// @brief 변수의 이름을 반환한다.
		/// @return 변수 이름
		SH_CORE_API auto GetName() const -> const core::Name&;
		/// @brief 해당 프로퍼티가 컨테이너라면 시작 반복자를 반환한다.
		/// @param SObject 프로퍼티 소유 객체
		/// @return 컨테이너가 아니라면 빈 반복자를 반환한다.
		SH_CORE_API auto Begin(SObject* SObject) -> PropertyIterator;
		SH_CORE_API auto Begin(SObject* SObject) const -> PropertyIterator;
		/// @brief 해당 프로퍼티가 컨테이너라면 시작 반복자를 반환한다.
		/// @param SObject 프로퍼티 소유 객체
		/// @return 컨테이너가 아니라면 빈 반복자를 반환한다.
		SH_CORE_API auto End(SObject* SObject) -> PropertyIterator;
		SH_CORE_API auto End(SObject* SObject) const -> PropertyIterator;

		/// @brief 프로퍼티가 컨테이너라면 얼마나 중첩된 컨테이너인지 반환한다.
		/// @return 1이면 단일 컨테이너, 컨테이너가 아니라면 0
		SH_CORE_API auto GetContainerNestedLevel() const -> uint32_t;
	};
}//namespace