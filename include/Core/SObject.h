#pragma once

#include "Export.h"
#include "Reflection.hpp"

#include <unordered_set>
#include <string>
#include <utility>

namespace sh::core
{
	class GC;
	class Observer;

	class SObject
	{
		SCLASS(SObject)
	private:
		GC* gc;

		bool bPendingKill;
	public:
		bool isHeap;

		SH_CORE_API SObject(GC* gc = nullptr);
		SH_CORE_API virtual ~SObject();
		SH_CORE_API void operator delete(void* ptr) noexcept;
		SH_CORE_API auto operator new(size_t size) -> void*;
		SH_CORE_API auto operator new[](size_t size) -> void* = delete;
		SH_CORE_API void operator delete[](void* ptr, size_t size) = delete;
		SH_CORE_API void SetGC(GC& gc);
		SH_CORE_API auto IsPendingKill() const -> bool;
#if SH_EDITOR
		SH_CORE_API virtual void OnPropertyChanged(const reflection::Property& prop);
#endif
	};
}