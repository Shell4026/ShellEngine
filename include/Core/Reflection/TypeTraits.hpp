#pragma once
#include "../Util.h"

#include <type_traits>
#include <array>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <string_view>
namespace sh::core
{
	class SObject;
}
namespace sh::core::reflection
{
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
	struct IsSObject : std::bool_constant<std::is_base_of_v<SObject, std::remove_reference_t<T>>> {};

	// 컨테이너 검증
	template<typename T, typename Check = void>
	struct IsContainer : std::bool_constant<false> {};
	template<typename T>
	struct IsContainer<T, std::void_t<decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())>> : std::bool_constant<true> {};

	template<typename T, typename Check = void>
	struct IsPair : std::bool_constant<false> {};
	template<typename T>
	struct IsPair<T, std::void_t<typename T::first_type, typename T::second_type>> : std::bool_constant<true> {};

	template<typename T>
	struct IsVector : std::bool_constant<false> {};
	template<typename T>
	struct IsVector<std::vector<T>> : std::bool_constant<true> {};

	template<typename T, typename U = void>
	struct IsMap : std::bool_constant<false> {};
	template<typename T, typename U>
	struct IsMap<std::map<T, U>, void> : std::bool_constant<true> {};

	template<typename T, typename U = void>
	struct IsHashMap : std::bool_constant<false> {};
	template<typename... Args>
	struct IsHashMap<std::unordered_map<Args...>> : std::bool_constant<true> {};

	template<typename T>
	struct IsSet : std::bool_constant<false> {};
	template<typename... Args>
	struct IsSet<std::set<Args...>> : std::bool_constant<true> {};

	template<typename T>
	struct IsHashSet : std::bool_constant<false> {};
	template<typename... Args>
	struct IsHashSet<std::unordered_set<Args...>> : std::bool_constant<true> {};

	template<typename T>
	struct IsArray : std::false_type {};
	template<typename T, std::size_t size>
	struct IsArray<std::array<T, size>> : std::true_type {};
	template<typename T, std::size_t size>
	struct IsArray<T[size]> : std::true_type {};

	template<typename T>
	struct IsUniquePtr : std::false_type {};
	template<typename T>
	struct IsUniquePtr<std::unique_ptr<T>> : std::true_type {};

	// 컨테이너 중첩 수 구하기
	template<typename T>
	struct GetContainerNestedCount : std::integral_constant<uint32_t, 0> {};
	template<typename T>
	struct GetContainerNestedCount<std::vector<T>> : std::integral_constant<uint32_t, GetContainerNestedCount<T>::value + 1> {};
	template<typename T, std::size_t N>
	struct GetContainerNestedCount<std::array<T, N>> : std::integral_constant<uint32_t, GetContainerNestedCount<T>::value + 1> {};
	template<typename T, std::size_t N>
	struct GetContainerNestedCount<T[N]> : std::integral_constant<uint32_t, GetContainerNestedCount<T>::value + 1> {};
	template<typename T, typename U, typename _Pr, typename _Alloc>
	struct GetContainerNestedCount<std::map<T, U, _Pr, _Alloc>> : std::integral_constant<uint32_t, GetContainerNestedCount<U>::value + 1> {};
	template<typename T, typename U, typename _Hasher, typename _Keyeq, typename _Alloc>
	struct GetContainerNestedCount<std::unordered_map<T, U, _Hasher, _Keyeq, _Alloc>> : std::integral_constant<uint32_t, GetContainerNestedCount<U>::value + 1> {};
	template<typename T, typename _Pr, typename _Alloc>
	struct GetContainerNestedCount<std::set<T, _Pr, _Alloc>> : std::integral_constant<uint32_t, GetContainerNestedCount<T>::value + 1> {};
	template<typename T, typename _Hasher, typename _Keyeq, typename _Alloc>
	struct GetContainerNestedCount<std::unordered_set<T, _Hasher, _Keyeq, _Alloc>> : std::integral_constant<uint32_t, GetContainerNestedCount<T>::value + 1> {};

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
	template<typename T, std::size_t n>
	struct GetContainerLastType<std::array<T, n>>
	{
		using type = typename GetContainerLastType<T>::type;
	};
	template<typename T, std::size_t n>
	struct GetContainerLastType<T[n]>
	{
		using type = T;
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

	template<typename T, typename = void>
	struct HasErase : std::bool_constant<false> {};
	template<typename T>
	struct HasErase<T, std::void_t<decltype(std::declval<T>().erase(std::declval<typename T::iterator>()))>> : std::bool_constant<true> {};

	class TypeTraits
	{
	private:
		template<typename T>
		static constexpr auto RawTypeName()
		{
#if defined(__clang__)
			return __PRETTY_FUNCTION__;
#elif defined(__GNUC__)
			return __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
			return __FUNCSIG__;
#else
#   error "Unsupported compiler"
#endif
		}
		template<typename T>
		static constexpr std::string_view TypePrefix()
		{
#if defined(__clang__)
			return "static constexpr auto sh::core::reflection::TypeTraits::RawTypeName() [with T = ";
#elif defined(__GNUC__)
			return "static constexpr auto sh::core::reflection::TypeTraits::RawTypeName() [with T = ";
#elif defined(_MSC_VER)
			return "auto __cdecl sh::core::reflection::TypeTraits::RawTypeName<";
#endif
		}
		template<typename T>
		static constexpr std::string_view TypeSuffix()
		{
#if defined(__clang__)
			return "]";
#elif defined(__GNUC__)
			return "]";
#elif defined(_MSC_VER)
			return ">(void)";
#endif
		}
		static constexpr auto CleanTypeName(std::string_view name) -> std::string_view
		{
			constexpr std::string_view classKeyword = "class ";
			constexpr std::string_view structKeyword = "struct ";
			if (name.find(classKeyword) != std::string_view::npos)
				name.remove_prefix(classKeyword.size());
			else if (name.find(structKeyword) != std::string_view::npos)
				name.remove_prefix(structKeyword.size());
			return name;
		}
	public:
		/// @brief 타입 이름(문자열)을 가져오는 함수
		/// @tparam T 타입
		template<typename T>
		static constexpr auto GetTypeName() -> std::string_view
		{
			constexpr std::string_view raw = RawTypeName<T>();
			constexpr std::string_view prefix = TypePrefix<T>();
			constexpr std::string_view suffix = TypeSuffix<T>();

			std::string_view name = raw;
			name.remove_prefix(prefix.size());
			name.remove_suffix(suffix.size());

			return CleanTypeName(name);
		}
		template<typename T>
		static constexpr auto GetTypeHash() -> std::size_t
		{
			std::string_view typeName = GetTypeName<T>();
			return Util::ConstexprHash(typeName);
		}
	};
}//namespace