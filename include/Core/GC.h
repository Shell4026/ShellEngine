#pragma once

#include "Export.h"

#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <queue>

namespace sh::core::reflection
{
	class PropertyIterator;
}

namespace sh::core
{
	class SObject;

	class GC
	{
	private:
		std::unordered_set<SObject*> objs;
		std::vector<SObject*> deletedObjs;
	private:
		//컨테이너를 재귀로 순회하면서 포인터를 검사하는 함수
		void DFSIteratorCheckPtr(int depth, int maxDepth, sh::core::reflection::PropertyIterator& it);
	public:
		SH_CORE_API void AddObject(SObject* obj);
		SH_CORE_API auto RemoveObject(SObject* obj) -> bool;
		SH_CORE_API void DeleteObject(SObject* obj);
		SH_CORE_API void Update();
	};
}