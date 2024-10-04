#pragma once

#include "Export.h"
#include "Reflection.hpp"

#include "Observer.hpp"

#include <string>
#include <utility>
#include <atomic>

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
		Observer<SObject*> onDestroy;
	public:
		SH_CORE_API SObject();
		SH_CORE_API SObject(const SObject& other);
		SH_CORE_API SObject(SObject&& other) noexcept;
		SH_CORE_API virtual ~SObject();
		SH_CORE_API auto operator new(std::size_t count) -> void*;
		SH_CORE_API void operator delete(void* ptr);
		SH_CORE_API auto IsPendingKill() const -> bool;
		SH_CORE_API auto IsMark() const -> bool;

		/// @brief GC에게 제거를 맡긴다.
		/// 
		/// @return 
		SH_CORE_API void Destroy();

		template<typename T>
		static auto Create() -> T*;
#if SH_EDITOR
		SH_CORE_API virtual void OnPropertyChanged(const reflection::Property& prop);
#endif
	};
}//namespace