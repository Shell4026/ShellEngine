#pragma once

#include "Export.h"
#include "UUID.h"
#include "SContainer.hpp"
#include "Singleton.hpp"

#include <cstdint>
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
		SHashMap<UUID, SObject*>& objs;
		SHashMap<SObject*, std::size_t> rootSetIdx;
		std::vector<SObject*> rootSets;

		uint32_t elapseTime = 0;
		uint32_t tick = 0;
		uint32_t updatePeriodTick = 1000;
		bool bContainerIteratorErased = false;
	private:
		/// @brief 중첩 컨테이너를 재귀로 순회하면서 SObject에 마킹 하는 함수
		/// @param bfs BFS용 큐
		/// @param parent 마킹이 시작된 오브젝트
		/// @param depth 현재 깊이
		/// @param maxDepth 최대 깊이
		/// @param it 넘길 반복자
		void ContainerMark(std::queue<SObject*>& bfs, SObject* parent, int depth, int maxDepth, sh::core::reflection::PropertyIterator& it);
		void Mark(std::size_t start, std::size_t end);
		void MarkMultiThread();
	protected:
		SH_CORE_API GarbageCollection();
	public:
		SH_CORE_API ~GarbageCollection();

		/// @brief 루트셋으로 지정하는 함수. 루트셋 객체는 참조하고 있는 객체가 없어도 메모리에서 유지된다.
		/// @param obj 루트셋으로 지정할 SObject 포인터
		SH_CORE_API void SetRootSet(SObject* obj);
		SH_CORE_API auto GetRootSet() const -> const std::vector<SObject*>&;
		/// @brief 해당 프레임마다 가비지 컬렉터를 수행한다.
		/// @param tick 목표 프레임
		SH_CORE_API void SetUpdateTick(uint32_t tick);

		/// @brief 루트셋에서 해당 객체를 제외하는 함수.
		/// @param obj SObject 포인터
		SH_CORE_API void RemoveRootSet(SObject* obj);

		/// @brief GC를 갱신하며 지정된 시간이 흐르면 Collect()가 호출 된다.
		SH_CORE_API void Update();
		/// @brief 쓰레기 수집 시작
		SH_CORE_API void Collect();

		/// @brief GC에 등록된 오브젝트 개수를 확인하는 함수
		/// @return GC에 등록된 SObject개수
		SH_CORE_API auto GetObjectCount() const -> std::size_t;

		/// @brief 강제로 메모리를 해제 하는 함수. 주의해서 써야한다. 해당 포인터를 참조하고 있던 값은 변하지 않는다.
		/// @param obj SObject 포인터
		SH_CORE_API void ForceDelete(SObject* obj);

		/// @brief 이전에 GC를 수행하는데 걸린 시간(ms)을 반환 하는 함수
		SH_CORE_API auto GetElapsedTime() -> uint32_t;
	};
}