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
		static constexpr const char* sync = "sync";
	};
}//namespace

namespace sh::core::reflection
{
	template<typename T, bool Constant>
	class IPropertyIterator;
	template<typename TContainer, typename T = typename TContainer::value_type, bool Constant = false>
	class PropertyIteratorData;
	template<bool Constant>
	class PropertyIterator;

	using PropertyIteratorT = PropertyIterator<false>;
	using ConstPropertyIteratorT = PropertyIterator<true>;

	/// @brief 자료형과 컨테이너를 숨긴 프로퍼티 반복자 인터페이스
	template<bool Constant>
	class IPropertyIteratorBase
	{
	public:
		virtual ~IPropertyIteratorBase() = default;
		virtual void operator++() = 0;
		virtual auto operator==(const IPropertyIteratorBase& other) -> bool = 0;
		virtual auto operator!=(const IPropertyIteratorBase& other) -> bool = 0;

		virtual auto GetNestedBegin() -> PropertyIterator<Constant> = 0;
		virtual auto GetNestedEnd() -> PropertyIterator<Constant> = 0;

		virtual auto IsPair() const -> bool = 0;
		virtual auto IsConst() const -> bool = 0;

		virtual auto GetPairFirst() const -> void* = 0;
		virtual auto GetPairSecond() const -> void* = 0;

		virtual void Erase() = 0;

		virtual auto GetType() const -> const TypeInfo& = 0;
		virtual auto GetPairType() const->std::optional<std::pair<TypeInfo, TypeInfo>> = 0;

		virtual auto Clone(void* buffer) const -> IPropertyIteratorBase* = 0;
	};
	//추상화된 프로퍼티 반복자
	template<bool Constant>
	class PropertyIterator
	{
	public:
		PropertyIterator()
		{
			std::memset(iteratorBuffer, 0, sizeof(iteratorBuffer));
		}
		template<typename TContainer, typename T = typename TContainer::value_type>
		PropertyIterator(const PropertyIteratorData<TContainer, T, Constant>& data)
		{
			static_assert(sizeof(PropertyIteratorData<TContainer, T, Constant>) <= sizeof(iteratorBuffer), "Iterator size exceeds buffer");
			iteratorDataPtr = new (iteratorBuffer) PropertyIteratorData<TContainer, T, Constant>{data};
		}
		PropertyIterator(const PropertyIterator& other)
		{
			other.iteratorDataPtr->Clone(iteratorBuffer);
			iteratorDataPtr = reinterpret_cast<IPropertyIteratorBase<Constant>*>(iteratorBuffer);
		}
		~PropertyIterator()
		{
			if (iteratorDataPtr != nullptr)
				iteratorDataPtr->~IPropertyIteratorBase<Constant>();
		}

		auto operator=(const PropertyIterator& other) -> PropertyIterator&
		{
			other.iteratorDataPtr->Clone(iteratorBuffer);
			iteratorDataPtr = reinterpret_cast<IPropertyIteratorBase<Constant>*>(iteratorBuffer);
			return *this;
		}
		auto operator==(const PropertyIterator& other) -> bool
		{
			return *iteratorDataPtr == *other.iteratorDataPtr;
		}
		auto operator!=(const PropertyIterator& other) -> bool
		{
			return *iteratorDataPtr != *other.iteratorDataPtr;
		}
		auto operator++() -> PropertyIterator&
		{
			++(*iteratorDataPtr);
			return *this;
		}

		/// @brief 원소의 타입을 반환 하는 함수
		/// @return 타입 객체
		auto GetType() const -> const TypeInfo&
		{
			return iteratorDataPtr->GetType();
		}
		/// @brief Pair타입 원소의 타입을 반환 하는 함수
		/// @return Pair가 아니라면 {}를 반환, 맞다면 Pair의 first와 second 타입을 반환
		auto GetPairType() const -> std::optional<std::pair<TypeInfo, TypeInfo>>
		{
			return iteratorDataPtr->GetPairType();
		}
		template<typename T>
		auto Get() -> std::conditional_t<Constant, const T*, T*>
		{
			//컨테이너 클래스가 아닌경우를 뜻한다.
			if (iteratorDataPtr == nullptr)
				return nullptr;

			auto itPtr = static_cast<IPropertyIterator<T, Constant>*>(iteratorDataPtr);
			return &itPtr->Get();
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
		auto GetNestedBegin() -> PropertyIteratorT
		{
			return iteratorDataPtr->GetNestedBegin();
		}
		auto GetNestedEnd() -> PropertyIteratorT
		{
			return iteratorDataPtr->GetNestedEnd();
		}

		auto IsPair() const -> bool
		{
			return iteratorDataPtr->IsPair();
		}
		/// @brief 원소가 const 변수인지 반환하는 함수.
		/// @return 맞으면 true, 아니면 false
		auto IsConst() const -> bool
		{
			return iteratorDataPtr->IsConst();
		}

		/// @brief 컨테이너에서 해당 반복자 위치의 원소를 지우는 함수.
		template<bool C = Constant, typename = typename std::enable_if_t<C == false>>
		void Erase()
		{
			iteratorDataPtr->Erase();
		}
	private:
		alignas(alignof(std::max_align_t)) uint8_t iteratorBuffer[48]; // 힙 할당을 최소화 하기 위해 도입, 메모리 정렬도 맞춰야 성능 보장
		IPropertyIteratorBase<Constant>* iteratorDataPtr = nullptr;
	};
	/// @brief 컨테이너 타입을 숨긴 프로퍼티 반복자 인터페이스
	/// @tparam T 컨테이너가 담고 있는 값의 타입
	template<typename T, bool Constant>
	class IPropertyIterator : public IPropertyIteratorBase<Constant>
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
	template<typename TContainer, typename T, bool Constant>
	class PropertyIteratorData : public IPropertyIterator<T, Constant>
	{
	private:
		using RawContainer = std::remove_cv_t<std::remove_reference_t<TContainer>>;
		using IteratorTypeT = std::conditional_t<std::is_const_v<TContainer>, typename RawContainer::const_iterator, typename RawContainer::iterator>;
		using IteratorReference = typename std::iterator_traits<IteratorTypeT>::reference;

		IteratorTypeT it;
		TContainer* container = nullptr;
	public:
		PropertyIteratorData(TContainer* container, IteratorTypeT initIterator) :
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
				return const_cast<void*>(reinterpret_cast<const void*>(&it->second));
		}

		/// @brief 내부 중첩된 컨테이너의 시작 반복자를 반환하는 함수
		/// @return 반복자
		auto GetNestedBegin() -> PropertyIterator<Constant> override
		{
			if constexpr (GetContainerNestedCount<TContainer>::value > 1)
			{
				if constexpr (sh::core::reflection::IsPair<T>::value)
				{
					using NestedType = typename T::second_type;
					using NestedValueT = typename std::remove_reference_t<NestedType>::value_type;
					return PropertyIterator<Constant>{ PropertyIteratorData<NestedType, NestedValueT, Constant>{ &it->second, it->second.begin() } };
				}
				else
				{
					using NestedContainerT = T; // T는 컨테이너 타입
					using NestedValueT = typename std::remove_reference_t<NestedContainerT>::value_type;
					return PropertyIterator<Constant>{ PropertyIteratorData<NestedContainerT, NestedValueT, Constant>{ &(*it), it->begin() } };
				}
			}
			return PropertyIterator<Constant>{};
		}
		/// @brief 내부 중첩된 컨테이너의 끝 반복자를 반환하는 함수
		/// @return 반복자
		auto GetNestedEnd() -> PropertyIterator<Constant> override
		{
			if constexpr (GetContainerNestedCount<TContainer>::value > 1)
			{
				if constexpr (sh::core::reflection::IsPair<T>::value)
				{
					using NestedType = typename T::second_type;
					using NestedValueT = typename std::remove_reference_t<NestedType>::value_type;
					return PropertyIterator<Constant>{ PropertyIteratorData<NestedType, NestedValueT, Constant>{ &it->second, it->second.end() } };
				}
				else
				{
					using NestedContainerT = T; // T는 컨테이너 타입
					using NestedValueT = typename std::remove_reference_t<NestedContainerT>::value_type;
					return PropertyIterator<Constant>{ PropertyIteratorData<NestedContainerT, NestedValueT, Constant>{ &(*it), it->end() } };
				}
			}
			return PropertyIterator<Constant>{};
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
			return std::is_const<std::remove_reference_t<IteratorReference>>::value;
		}

		auto operator==(const IPropertyIteratorBase<Constant>& other) -> bool override
		{
			return static_cast<const PropertyIteratorData<TContainer, T, Constant>*>(&other)->it == it;
		}

		auto operator!=(const IPropertyIteratorBase<Constant>& other) -> bool override
		{
			return static_cast<const PropertyIteratorData<TContainer, T, Constant>*>(&other)->it != it;
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
				if constexpr (std::is_pointer_v<T> && !std::is_const_v<std::remove_reference_t<IteratorReference>>)
					*it = nullptr;
			}
		}

		auto Clone(void* buffer) const -> IPropertyIteratorBase<Constant>* override
		{
			new (buffer) PropertyIteratorData<TContainer, T, Constant>{ *this };
			return reinterpret_cast<PropertyIteratorData<TContainer, T, Constant>*>(buffer);
		}
	};

	/// @brief 타입을 숨긴 프로퍼티 추상 클래스
	class PropertyDataBase
	{
	public:
		virtual auto GetType() const -> const TypeInfo& = 0;
		virtual auto Begin(void* sobject) const -> PropertyIteratorT = 0;
		virtual auto Begin(const void* sobject) const -> ConstPropertyIteratorT = 0;
		virtual auto End(void* sobject) const -> PropertyIteratorT = 0;
		virtual auto End(const void* sobject) const -> ConstPropertyIteratorT = 0;
		virtual void ClearContainer(void* sobject) const = 0;
		virtual void InsertToContainer(void* sobject, const void* value) const = 0;
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
		auto Begin(void* sobject) const -> PropertyIteratorT override
		{
			if constexpr (IsContainer<T>())
			{
				if constexpr (std::is_member_pointer_v<VariablePointer>)
				{
					T& container = static_cast<ThisType*>(sobject)->*ptr;
					return PropertyIteratorT{ PropertyIteratorData<T>{ &container, container.begin() } };
				}
				else
				{
					T& container = *ptr;
					return PropertyIteratorT{ PropertyIteratorData<T>{ &container, container.begin() } };
				}
			}
			else
				return PropertyIteratorT{};
		}
		auto Begin(const void* sobject) const -> ConstPropertyIteratorT override
		{
			if constexpr (IsContainer<T>())
			{
				if constexpr (std::is_member_pointer_v<VariablePointer>)
				{
					const T& container = static_cast<const ThisType*>(sobject)->*ptr;
					using ContainerType = std::remove_reference_t<decltype(container)>;
					using ValueType = typename std::remove_reference_t<ContainerType>::value_type;
					return ConstPropertyIteratorT{ PropertyIteratorData<const ContainerType, const ValueType, true>{ &container, container.begin() } };
				}
				else
				{
					const T& container = *ptr;
					using ContainerType = std::remove_reference_t<decltype(container)>;
					using ValueType = typename std::remove_reference_t<ContainerType>::value_type;
					return ConstPropertyIteratorT{ PropertyIteratorData<const ContainerType, const ValueType, true>{ &container, container.begin() } };
				}
			}
			else
				return ConstPropertyIteratorT{};
		}

		/// @brief 해당 프로퍼티가 컨테이너라면 시작 반복자를 반환한다.
		/// @param sobject 해당 프로퍼티를 가지고 있는 클래스 포인터
		/// @return 컨테이너라면 유효한 반복자를, 아니라면 빈 반복자를 반환한다.
		auto End(void* sobject) const -> PropertyIteratorT override
		{
			if constexpr (IsContainer<T>())
			{
				if constexpr (std::is_member_pointer_v<VariablePointer>)
				{
					T& container = static_cast<ThisType*>(sobject)->*ptr;
					return PropertyIteratorT{ PropertyIteratorData<T>{ &container, container.end() } };
				}
				else
				{
					T& container = *ptr;
					return PropertyIteratorT{ PropertyIteratorData<T>{ &container, container.end() } };
				}
			}
			else
				return PropertyIteratorT{};
		}
		auto End(const void* sobject) const -> ConstPropertyIteratorT override
		{
			if constexpr (IsContainer<T>())
			{
				if constexpr (std::is_member_pointer_v<VariablePointer>)
				{
					const T& container = static_cast<const ThisType*>(sobject)->*ptr;
					using ContainerType = std::remove_reference_t<decltype(container)>;
					using ValueType = typename std::remove_reference_t<ContainerType>::value_type;
					return ConstPropertyIteratorT{ PropertyIteratorData<const ContainerType, const ValueType, true>{ &container, container.end() } };
				}
				else
				{
					const T& container = *ptr;
					using ContainerType = std::remove_reference_t<decltype(container)>;
					using ValueType = typename std::remove_reference_t<ContainerType>::value_type;
					return ConstPropertyIteratorT{ PropertyIteratorData<const ContainerType, const ValueType, true>{ &container, container.end() } };
				}
			}
			else
				return ConstPropertyIteratorT{};
		}

		void ClearContainer(void* SObjectPtr) const override
		{
			if constexpr (IsContainer<T>())
			{
				if constexpr (std::is_member_pointer_v<VariablePointer>)
				{
					T& container = static_cast<ThisType*>(SObjectPtr)->*ptr;
					if constexpr (HasClear<T>::value)
						container.clear();
				}
				else
				{
					T& container = *ptr;
					if constexpr (HasClear<T>::value)
						container.clear();
				}
			}
		}
		void InsertToContainer(void* obj, const void* value) const
		{
			if constexpr (IsContainer<T>() && !std::is_const_v<T>)
			{
				if constexpr (std::is_same_v<T, std::string>)
				{
					std::string& str = static_cast<ThisType*>(obj)->*ptr;
					str.push_back(*reinterpret_cast<const char*>(value));
				}
				else if constexpr (IsVector<T>())
				{
					T& vector = static_cast<ThisType*>(obj)->*ptr;
					using ValueType = typename GetContainerElementType<T>::type;

					vector.push_back(*reinterpret_cast<const ValueType*>(value));
				}
				else if constexpr (IsSet<T>() || IsHashSet<T>())
				{
					T& set = static_cast<ThisType*>(obj)->*ptr;
					using ValueType = typename GetContainerElementType<T>::type;

					set.insert(*reinterpret_cast<const ValueType*>(value));
				}
				else if constexpr (IsMap<T>() || IsHashMap<T>())
				{
					T& map = static_cast<ThisType*>(obj)->*ptr;
					using PairType = typename GetContainerElementType<T>::type;

					auto pairPtr = reinterpret_cast<const PairType*>(value);
					map.insert_or_assign(pairPtr->first, pairPtr->second);
				}
				else if constexpr (IsList<T>())
				{
					T& list = static_cast<ThisType*>(obj)->*ptr;
					using ValueType = typename GetContainerElementType<T>::type;

					list.push_back(*reinterpret_cast<const ValueType*>(value));
				}
			}
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

		static auto ParseOption(const std::initializer_list<std::string_view>& options) -> Option
		{
			Option retOption{};
			for (auto& option : options)
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
		PropertyCreateInfo(std::string_view name, const std::initializer_list<std::string_view>& options) :
			name(name), option(ParseOption(options))
		{
		}
	};

	/// @brief 프로퍼티 클래스
	class Property
	{
	public:
		template<typename ThisType, typename T, typename VariablePointer, VariablePointer ptr>
		Property(const PropertyCreateInfo<ThisType, T, VariablePointer, ptr>& createInfo) :
			type(GetType<T>()),
			pureTypeName(TypeTraits::GetTypeName<std::remove_const_t<std::remove_pointer_t<std::remove_reference_t<T>>>>()),
			name(createInfo.name),
			containerNestedLevel(GetContainerNestedCount<T>::value),
			containerElementType(IsContainer<T>::value ? &GetType<typename GetContainerElementType<T>::type>() : nullptr),
			bConstProperty(createInfo.option.bConst),
			bVisibleProperty(createInfo.option.bVisible),
			bNoSaveProperty(createInfo.option.bNoSave),
			isConst(std::is_const_v<T>),
			isPointer(std::is_pointer_v<T>),
			isContainer(IsContainer<T>::value),
			isSObject(IsSObject<T>::value),
			isSObjectPointer(std::is_convertible_v<T, const SObject*>),
			isSObjectPointerContainer(reflection::IsContainer<T>::value && std::is_convertible_v<typename reflection::GetContainerLastType<T>::type, const SObject*>),
			isEnum(std::is_enum_v<T>)
		{
			// 메모) 템플릿 인자로 인해 클래스 맴버 변수 별로 메모리 상에 하나만 존재하게 된다.
			static PropertyData<ThisType, T, VariablePointer, ptr> data{};
			this->data = &data;
		}
		SH_CORE_API Property(const Property& other) noexcept;

		SH_CORE_API auto operator==(const Property& other) -> bool;
		SH_CORE_API auto operator==(const TypeInfo& type) -> bool;
		SH_CORE_API auto operator!=(const Property& other) -> bool;
		SH_CORE_API auto operator!=(const TypeInfo& type) -> bool;

		/// @brief 프로퍼티가 가지고 있는 값을 반환하는 함수.
		/// @brief 주의: 타입 검사를 하지 않음.
		/// @tparam T 반환 타입
		/// @param sobject 해당 프로퍼티를 가지고 있는 SObject객체
		/// @return 실제 값
		template<typename T>
		auto Get(SObject& sobject) const -> T*
		{
			assert(isConst ? std::is_const_v<T> : true);
			return &static_cast<IPropertyData<T>*>(data)->Get(&sobject);
		}
		template<typename T>
		auto Get(const SObject& sobject) const -> const T*
		{
			return &static_cast<IPropertyData<T>*>(data)->Get(&sobject);
		}
		template<typename T>
		auto GetSafe(SObject& sobject) const -> T*
		{
			if (data->GetType() != reflection::GetType<T>())
				return nullptr;
 			if (isConst ? !std::is_const_v<T> : false)
				return nullptr;
			return Get<T>(sobject);
		}
		template<typename T>
		auto GetSafe(const SObject& sobject) const -> const T*
		{
			if (data->GetType() != reflection::GetType<T>())
				return nullptr;
			return Get<T>(sobject);
		}
		/// @brief 변수의 이름을 반환한다.
		/// @return 변수 이름
		SH_CORE_API auto GetName() const -> const core::Name&;
		/// @brief 해당 프로퍼티가 컨테이너라면 시작 반복자를 반환한다.
		/// @param SObject 프로퍼티 소유 객체
		/// @return 컨테이너가 아니라면 빈 반복자를 반환한다.
		SH_CORE_API auto Begin(SObject& SObject) const -> PropertyIteratorT;
		SH_CORE_API auto Begin(const SObject& SObject) const-> ConstPropertyIteratorT;
		/// @brief 해당 프로퍼티가 컨테이너라면 시작 반복자를 반환한다.
		/// @param SObject 프로퍼티 소유 객체
		/// @return 컨테이너가 아니라면 빈 반복자를 반환한다.
		SH_CORE_API auto End(SObject& SObject) const -> PropertyIteratorT;
		SH_CORE_API auto End(const SObject& SObject) const -> ConstPropertyIteratorT;
		/// @brief 프로퍼티가 컨테이너라면 얼마나 중첩된 컨테이너인지 반환한다.
		/// @return 1이면 단일 컨테이너, 컨테이너가 아니라면 0
		SH_CORE_API auto GetContainerNestedLevel() const -> uint32_t;
		/// @brief 프로퍼티가 컨테이너라면 컨테이너를 비운다.
		/// @return 컨테이너라면 true, 아니라면 false
		SH_CORE_API auto ClearContainer(SObject& SObject) const -> bool;
		/// @brief 프로퍼티가 컨테이너라면 값을 넣는다. 아니라면 아무 동작도 일어나지 않는다. 
		/// @brief map의 경우 pair를 넣으면 된다. 
		/// @brief 컨테이너 원소의 타입으로 자동 형변환이 되지 않으므로 정확한 타입의 데이터를 넣어야 한다.
		/// @brief [주의] 타입 검사를 하지 않는다.
		/// @tparam T 타입
		/// @param value 값
		/// @param SObject SObject 프로퍼티 소유 객체
		template<typename T>
		void InsertToContainer(SObject& SObject, const T& value) const
		{
			data->InsertToContainer(&SObject, &value);
		}
	public:
		const TypeInfo& type;
		const TypeInfo* const containerElementType;
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
		const bool isEnum;
	private:
		PropertyDataBase* data;

		core::Name name;

		const uint32_t containerNestedLevel;
	};
}//namespace