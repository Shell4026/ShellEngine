#pragma once
#include "Export.h"
#include "Reflection.hpp"
#include "ISerializable.h"
#include "SObjectManager.h"
#include "Observer.hpp"
#include "UUID.h"
#include "Name.h"

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
		Name name;
		std::atomic_flag bMark;
		std::atomic<bool> bPendingKill;
	protected:
		bool bPlacementNew = false;
	public:
		mutable Observer<false, const SObject*> onDestroy;
	private:
		SH_CORE_API static void RegisterToManager(SObject* ptr);
	protected:
		SH_CORE_API auto operator new(std::size_t count) -> void*;
		SH_CORE_API auto operator new(std::size_t count, void* ptr) -> void*;
		SH_CORE_API void operator delete(void* ptr);

		SH_CORE_API SObject();
	public:
		/// @brief 복사 생성자. UUID는 복사하지 않는다.
		/// @param other 다른 객체
		SH_CORE_API SObject(const SObject& other);
		SH_CORE_API SObject(SObject&& other) noexcept;
		SH_CORE_API virtual ~SObject();
		SH_CORE_API auto IsPendingKill() const -> bool;

		/// @brief GC에게 제거를 맡긴다.
		SH_CORE_API virtual void Destroy();
		/// @brief GC에서 소멸 되기전에 호출된다.
		SH_CORE_API virtual void OnDestroy();
		SH_CORE_API virtual void OnPropertyChanged(const reflection::Property& prop);

		SH_CORE_API void SetName(std::string_view name);
		SH_CORE_API void SetName(const core::Name& name);
		SH_CORE_API auto GetName() const -> const Name&;

		/// @brief UUID를 재설정한다. 이미 존재하는 UUID로 설정 시 실패한다.
		/// @param uuid UUID 객체
		/// @return 성공 시 true, 실패 시 false
		SH_CORE_API auto SetUUID(const UUID& uuid) -> bool;
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
		/// @brief SObject 객체를 이미 할당 된 메모리의 주소에 생성한다. 생성시 GC에 등록되며 사용되지 않을 시 소멸된다. 
		/// 메모리는 해제 되지 않고 소멸자만 호출된다.
		/// @tparam T SObject를 상속 받는 객체
		/// @param ptr 생성 할 주소
		/// @param ...args 생성자에 전달할 인자
		/// @return 객체 포인터
		template<typename T, typename... Args>
		static auto CreateAt(void* posPtr, Args&&... args) -> std::enable_if_t<std::is_base_of_v<SObject, T>, T*>
		{
			SObject* ptr = new (posPtr) T(std::forward<Args>(args)...);
			RegisterToManager(ptr);
			ptr->bPlacementNew = true;
			return static_cast<T*>(ptr);
		}
	};

	template<>
	inline void SerializeProperty(core::Json& json, const std::string& key, SObject* const& value)
	{
		if (value != nullptr)
			json[key] = value->GetUUID().ToString();
	}
	template<>
	inline void DeserializeProperty(const core::Json& json, const std::string& key, SObject*& value)
	{
		if (json.contains(key))
		{
			std::string uuid = json[key].get<std::string>();
			auto objectManager = SObjectManager::GetInstance();
			SObject* ptr = objectManager->GetSObject(UUID{ uuid });
			if (ptr == nullptr)
				return;

			value = ptr;
		}
	}
}//namespace