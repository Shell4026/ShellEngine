#pragma once

#include "Export.h"

#include <unordered_set>
#include <unordered_map>
#include <vector>
namespace sh::core
{
	class SObject;

	class GC
	{
	private:
		std::unordered_set<SObject*> objs;
	public:
		SH_CORE_API void AddObject(SObject* obj);
		SH_CORE_API void RemoveObject(SObject* obj);
		SH_CORE_API void DeleteObject(SObject* obj);
	};
}