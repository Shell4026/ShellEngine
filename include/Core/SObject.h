﻿#pragma once

#include "Export.h"
#include "Reflection.hpp"
#include "ISerializable.h"
#include "SObjectManager.h"
#include "Observer.hpp"
#include "UUID.h"

#include <string>
#include <utility>
#include <atomic>
#include <type_traits>

namespace sh::core
{
	class GarbageCollection;

	/// @brief 엔진에서 쓰는 기본 객체. 리플렉션과 가비지 컬렉터를 쓰려면 해당 객체를 상속해야한다.
	class SObject : public ISerializable
	{
		SCLASS(SObject)

		friend GarbageCollection;
	private:
		UUID uuid;
		std::string name{};
		std::atomic<bool> bPendingKill;
		bool bMark;
	public:
		mutable Observer<false, const SObject*> onDestroy;
	private:
		SH_CORE_API static void RegisterToManager(SObject* ptr);
	protected:
		SH_CORE_API auto operator new(std::size_t count) -> void*;
		SH_CORE_API void operator delete(void* ptr);

		SH_CORE_API SObject();
	public:
		SH_CORE_API SObject(const SObject& other);
		SH_CORE_API SObject(SObject&& other) noexcept;
		SH_CORE_API virtual ~SObject();
		SH_CORE_API auto IsPendingKill() const -> bool;
		/// @brief GC에서 마킹 됐는지 확인하는 함수
		/// @return 마킹 여부
		SH_CORE_API auto IsMark() const -> bool;

		/// @brief GC에게 제거를 맡긴다.
		SH_CORE_API virtual void Destroy();
		/// @brief GC에서 소멸 되기전에 호출된다.
		SH_CORE_API virtual void OnDestroy();
		SH_CORE_API virtual void OnPropertyChanged(const reflection::Property& prop);

		SH_CORE_API void SetName(std::string_view name);
		SH_CORE_API auto GetName() const -> const std::string&;

		SH_CORE_API void SetUUID(const UUID& uuid);
		SH_CORE_API auto GetUUID() const -> const UUID&;

		SH_CORE_API auto Serialize() const -> Json override;
		SH_CORE_API void Deserialize(const Json& json) override;

		/// @brief SObject 객체를 생성한다. 생성시 GC에 등록되며 사용되지 않을 시 소멸된다.
		/// @tparam T SObject를 상속 받는 객체
		/// @param ...args 생성자에 전달할 인자
		/// @return 객체 포인터
		template<typename T, typename... Args>
		static auto Create(Args&&... args) -> std::enable_if_t<std::is_base_of_v<SObject, T>, T*>
		{
			SObject* ptr = new T(std::forward<Args>(args)...);
			RegisterToManager(ptr);
			return static_cast<T*>(ptr);
		}
	};

	template<>
	inline void SerializeProperty(core::Json& json, const std::string& key, SObject* const& value)
	{
		json[key] = value->GetUUID().ToString();
	}
	template<>
	inline void DeserializeProperty(const core::Json& json, const std::string& key, SObject*& value)
	{
		if (json.contains(key))
		{
			std::string uuid = json[key].get<std::string>();
			auto objectManager = SObjectManager::GetInstance();
			SObject* ptr = objectManager->GetSObject(uuid);
			if (ptr == nullptr)
				return;

			value = ptr;
		}
	}
}//namespace