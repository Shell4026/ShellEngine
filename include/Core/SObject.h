#pragma once

#include "Export.h"
#include "Reflection.hpp"

#include <vector>
#include <unordered_map>
#include <string>
#include <utility>

namespace sh::core
{
	class GC;

	class SObject
	{
		SCLASS(SObject)
	private:
		GC* gc;
	public:
		SH_CORE_API SObject(GC* gc = nullptr);
		SH_CORE_API ~SObject();

		SH_CORE_API void SetGC(GC& gc);
	};
}