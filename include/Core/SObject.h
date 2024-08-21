#pragma once

#include "Export.h"
#include "Reflection.hpp"

#include "Memory/SAllocator.hpp"

#include <unordered_set>
#include <string>
#include <utility>
#include <atomic>

namespace sh::core
{
	class Observer;
	class GarbageCollection;

	class SObject
	{
		SCLASS(SObject)

		friend GarbageCollection;
	private:
		std::unordered_set<Observer*> destroyObservers;

		std::atomic<bool> bPendingKill;
		bool bMark;
	protected:
		GarbageCollection* gc;
	public:
		SH_CORE_API SObject();
		SH_CORE_API SObject(const SObject& other);
		SH_CORE_API SObject(SObject&& other) noexcept;
		SH_CORE_API virtual ~SObject();
		SH_CORE_API auto operator new(std::size_t size) -> void*;
		SH_CORE_API void operator delete(void* ptr, std::size_t size);
		SH_CORE_API auto IsPendingKill() const -> bool;
		SH_CORE_API auto IsMark() const -> bool;

		SH_CORE_API void RegisterDestroyNotify(Observer& observer);
		SH_CORE_API void UnRegeisterDestroyNotify(Observer& observer);

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

	template<typename T>
	inline auto SObject::Create() -> T*
	{
		return nullptr;
	}
}//namespace