#pragma once

#include "Export.h"

#include "SContainer.hpp"
#include "Singleton.hpp"

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
	
	/// @brief 가비지 컬렉터
	class GarbageCollection : public Singleton<GarbageCollection>
	{
		friend Singleton<GarbageCollection>;
	private:
		SHashSet<SObject*, 128> objs;
		SHashSet<SObject*, 128> rootSets;
		SHashSet<SObject*, 16> deletedObjs;
	private:
		//컨테이너를 재귀로 순회하면서 포인터를 검사하는 함수
		void DFSIteratorCheckPtr(SObject* target, int depth, int maxDepth, sh::core::reflection::PropertyIterator& it);

		void Mark(SObject* obj, SObject* parent);
	protected:
		SH_CORE_API GarbageCollection();
	public:
		SH_CORE_API ~GarbageCollection();

		/// @brief GC 목록에 추가하는 함수.
		/// @param obj SObject 포인터
		/// @return 
		SH_CORE_API void AddObject(SObject* obj);
		/// @brief 루트셋으로 지정하는 함수. 루트셋 객체는 참조하고 있는 객체가 없어도 메모리에서 유지된다.
		/// @param obj 루트셋으로 지정할 SObject 포인터
		/// @return 
		SH_CORE_API void SetRootSet(SObject* obj);

		/// @brief 루트셋에서 해당 객체를 제외하는 함수.
		/// @param obj SObject 포인터
		/// @return 
		SH_CORE_API void RemoveRootSet(SObject* obj);

		/// @brief GC 목록에서 제거하는 함수.
		/// @param obj SObject 포인터
		/// @return 성공하면 true, 아니면 false
		SH_CORE_API auto RemoveObject(SObject* obj) -> bool;

		/// @brief 강제 제거(delete) 했을 때 GC에 알리는 함수.
		/// @param obj SObject 포인터
		/// @return 
		SH_CORE_API void DeleteObject(SObject* obj);

		/// @brief GC를 갱신하여 쓰레기 수집 시작
		/// @return 
		SH_CORE_API void Update();

		/// @brief GC에 등록된 오브젝트 개수를 확인하는 함수
		/// @return GC에 등록된 SObject개수
		SH_CORE_API auto GetObjectCount() const -> std::size_t;
	};
}