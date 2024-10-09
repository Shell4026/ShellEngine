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
	class Property;
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

		bool bContainerIteratorErased;
	private:
		/// @brief 중첩 컨테이너를 재귀로 순회하면서 SObject에 마킹 하는 함수
		/// @param parent 마킹이 시작된 오브젝트
		/// @param depth 현재 깊이
		/// @param maxDepth 최대 깊이
		/// @param it 넘길 반복자
		void ContainerMark(SObject* parent, int depth, int maxDepth, sh::core::reflection::PropertyIterator& it);
		/// @brief 마킹 함수
		/// @param obj 마킹 할 객체
		/// @param parent 마킹 할 객체의 부모
		/// @param parentProperty 부모의 해당 객체를 가르키는 프로퍼티
		/// @param parentIterator 부모 컨테이너의 해당 객체를 가르키는 반복자 프로퍼티
		void Mark(SObject* obj, SObject* parent, core::reflection::Property* parentProperty, core::reflection::PropertyIterator* parentIterator);
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
		SH_CORE_API void SetRootSet(SObject* obj);

		/// @brief 루트셋에서 해당 객체를 제외하는 함수.
		/// @param obj SObject 포인터
		/// @return 
		SH_CORE_API void RemoveRootSet(SObject* obj);

		/// @brief GC 목록에서 제거하는 함수.
		/// @param obj SObject 포인터
		/// @return 성공하면 true, 아니면 false
		SH_CORE_API auto RemoveObject(SObject* obj) -> bool;

		/// @brief GC를 갱신하여 쓰레기 수집 시작
		SH_CORE_API void Update();

		/// @brief GC에 등록된 오브젝트 개수를 확인하는 함수
		/// @return GC에 등록된 SObject개수
		SH_CORE_API auto GetObjectCount() const -> std::size_t;
	};
}