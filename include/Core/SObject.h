#pragma once

#include "Export.h"
#include "Reflection.hpp"

#include "Observer.hpp"

#include <string>
#include <utility>
#include <atomic>
#include <type_traits>

namespace sh::core
{
	class GarbageCollection;

	/// @brief 엔진에서 쓰는 기본 객체. 리플렉션과 가비지 컬렉터를 쓰려면 해당 객체를 상속해야한다.
	class SObject
	{
		SCLASS(SObject)

		friend GarbageCollection;
	private:
		std::atomic<bool> bPendingKill;
		bool bMark;
	protected:
		GarbageCollection* gc;
	public:
		Observer<false, SObject*> onDestroy;
#if SH_EDITOR
		std::string editorName;
#endif
	protected:
		SH_CORE_API static void SetGC(SObject* ptr);
		SH_CORE_API auto operator new(std::size_t count) -> void*;
		SH_CORE_API void operator delete(void* ptr);
	public:
		SH_CORE_API SObject();
		SH_CORE_API SObject(const SObject& other);
		SH_CORE_API SObject(SObject&& other) noexcept;
		SH_CORE_API virtual ~SObject();
		SH_CORE_API auto IsPendingKill() const -> bool;
		SH_CORE_API auto IsMark() const -> bool;

		/// @brief GC에게 제거를 맡긴다.
		SH_CORE_API void Destroy();
		/// @brief GC에서 소멸 되기전에 호출된다.
		SH_CORE_API virtual void OnDestroy();
		/// @brief SObject 객체를 생성한다. 생성시 GC에 등록되며 사용되지 않을 시 소멸된다.
		/// @tparam T SObject를 상속 받는 객체
		/// @param ...args 생성자에 전달할 인자
		/// @return 객체 포인터
		template<typename T, typename... Args>
		static auto Create(Args&&... args) -> std::enable_if_t<std::is_base_of_v<SObject, T>, T*>
		{
			SObject* ptr = new T(std::forward<Args>(args)...);
			SetGC(ptr);
			return static_cast<T*>(ptr);
		}
#if SH_EDITOR
		SH_CORE_API virtual void OnPropertyChanged(const reflection::Property& prop);
#endif
	};
}//namespace