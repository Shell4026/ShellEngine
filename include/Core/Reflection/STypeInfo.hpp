#pragma once
#include "Export.h"
#include "TypeTraits.hpp"
#include "Property.hpp"
#include "../Name.h"

#include <string_view>
#include <unordered_map>

/// SCLASS 매크로
/// 해당 매크로를 선언하면 해당 클래스는 리플렉션 데이터를 가지게 된다.
#define SCLASS(class_name)\
private:\
	inline static sh::core::reflection::STypeInfo* stypeInfo = nullptr;\
public:\
	using Super = sh::core::reflection::MakeSuper<class_name>::type;\
	using This = class_name;\
public:\
	static auto GetStaticType() -> sh::core::reflection::STypeInfo&\
	{\
		if(stypeInfo == nullptr)\
		{\
			std::size_t hash = typeid(class_name).hash_code();\
			auto it = sh::core::reflection::STypes::types.find(hash);\
			if (it == sh::core::reflection::STypes::types.end())\
			{\
				static sh::core::reflection::STypeInfo info{ sh::core::reflection::STypeCreateInfo<class_name>{ #class_name, hash } }; \
				sh::core::reflection::STypes::types.insert({ hash, &info });\
				stypeInfo = &info;\
				return info;\
			}\
			else\
			{\
				stypeInfo = it->second;\
				return *stypeInfo;\
			}\
		}\
		else\
			return *stypeInfo;\
	}\
	virtual auto GetType() const -> sh::core::reflection::STypeInfo&\
	{\
		if (stypeInfo == nullptr)\
			stypeInfo = &GetStaticType();\
		return *stypeInfo;\
	}

namespace sh::core::reflection
{
	class STypeInfo;

	template<typename T, typename U = void>
	struct MakeSuper {
		using type = U;
	};
	template<typename T>
	struct MakeSuper<T, std::void_t<typename T::This>> {
		using type = typename T::This;
	};

	/// @brief TypeInfo를 만드는데 필요한 구조체
	/// @tparam T 타입
	template<typename T>
	struct STypeCreateInfo
	{
		using Type = T;

		const std::string_view name;
		STypeInfo* super;
		const std::size_t hash;

		STypeCreateInfo(std::string_view name, std::size_t hash) :
			name(name), super(nullptr), hash(hash)
		{
			if constexpr (HasSuper<T>())
			{
				super = &(T::Super::GetStaticType());
			}
		}
	};

	/// @brief SClass의 타입 정보 객체
	class STypeInfo
	{
	private:
		const size_t hash;
		STypeInfo* super;

		std::vector<std::unique_ptr<Property>> properties;
		std::vector<Property*> sobjPtrs;
		std::vector<Property*> sobjPtrContainers;
	public:
		const core::Name name;
		const TypeInfo& type;
		const size_t size;
		const bool isPointer;
	public:
		template<typename T>
		explicit STypeInfo(STypeCreateInfo<T> data) :
			hash(data.hash), super(data.super),
			name(data.name), type(GetType<T>()), size(sizeof(T)), isPointer(std::is_pointer_v<T>)
		{
		}
		STypeInfo(const STypeInfo& other) = delete;

		SH_CORE_API auto AddProperty(const Property& prop) -> Property*;

		SH_CORE_API auto operator==(const STypeInfo& other) const -> bool;
		SH_CORE_API auto operator!=(const STypeInfo& other) const -> bool;

		SH_CORE_API auto GetSuper() const -> STypeInfo*;
		/// @brief 현재 타입이 other의 자식인지
		/// @return 자식이면 true 아니면 false
		SH_CORE_API auto IsChildOf(const STypeInfo& other) const -> bool;

		SH_CORE_API auto GetProperty(const core::Name& name) const -> Property*;
		SH_CORE_API auto GetProperty(std::string_view name) const->Property*;
		SH_CORE_API auto GetProperties() const -> const std::vector<std::unique_ptr<Property>>&;
		SH_CORE_API auto GetSObjectPtrProperties() const -> const std::vector<Property*>&;
		SH_CORE_API auto GetSObjectPtrContainerProperties() const -> const std::vector<Property*>&;
	};//STypeInfo

	/// @brief 리플렉션 데이터의 DLL간 공유를 위한 구조체
	struct STypes
	{
		SH_CORE_API static std::unordered_map<std::size_t, STypeInfo*> types; // 해쉬, 타입 포인터
	};
}//namespace