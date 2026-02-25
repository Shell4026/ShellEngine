#pragma once
#include "Export.h"
#include "Singleton.hpp"
#include "SObject.h"

#include <cstdint>
#include <algorithm>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <array>
#include <vector>
#include <queue>
#include <mutex>
#include <variant>
#include <cstring>
#include <list>
#include <functional>
namespace sh::core::reflection
{
	class Property;
	template<bool Constant>
	class PropertyIterator;
}

namespace sh::core
{
	template<typename T, typename IsSObject = std::enable_if_t<std::is_base_of_v<SObject, T>>>
	class SObjWeakPtr;
	
	class GCObject;

	/// @brief 마크 앤 스윕기반 가비지 컬렉터
	class GarbageCollection : public Singleton<GarbageCollection>
	{
		friend Singleton<GarbageCollection>;
	public:
		SH_CORE_API ~GarbageCollection();

		/// @brief 루트셋으로 지정하는 함수. 루트셋 객체는 참조하고 있는 객체가 없어도 메모리에서 유지된다.
		/// @param obj 루트셋으로 지정할 SObject 포인터
		SH_CORE_API void SetRootSet(SObject* obj);
		/// @brief 루트셋에서 해당 객체를 제외하는 함수.
		/// @param obj SObject 포인터
		SH_CORE_API void RemoveRootSet(const SObject* obj);
		/// @brief 해당 프레임마다 가비지 컬렉터를 수행한다.
		/// @param tick 목표 프레임
		SH_CORE_API void SetUpdateTick(uint32_t tick);

		/// @brief GC를 갱신하며 지정된 시간이 흐르면 Collect()와 DestroyPendingKillObjs()가 호출 된다.
		SH_CORE_API void Update();
		/// @brief 쓰레기 수집 시작
		SH_CORE_API void Collect();

		/// @brief GC에 등록된 오브젝트 개수를 확인하는 함수
		/// @return GC에 등록된 SObject개수
		SH_CORE_API auto GetObjectCount() const -> std::size_t;

		/// @brief 강제로 메모리를 해제 하는 함수. 주의해서 써야한다. 해당 포인터를 참조하고 있던 값은 변하지 않는다.
		/// @param obj SObject 포인터
		SH_CORE_API void ForceDelete(SObject* obj);
		/// @brief 삭제 보류 목록에 오브젝트를 추가한다.
		/// @param obj 오브젝트
		SH_CORE_API void AddToPendingKillList(SObject* obj);
		/// @brief 삭제 보류중인 객체들을 즉시 메모리에서 제거한다.
		SH_CORE_API void DestroyPendingKillObjs();

		/// @brief bfs큐를 돌며 SObject를 마킹 하는 함수.
		/// @brief 외부에서는 TrackedContainer의 fn함수 내에서만 사용해야 한다.
		SH_CORE_API void MarkBFS(std::queue<SObject*>& bfs);

		SH_CORE_API void AddGCObject(GCObject& obj);
		SH_CORE_API void RemoveGCObject(GCObject& obj);

		SH_CORE_API auto GetRootSet() const -> const std::vector<SObject*>& { return rootSets; }
		SH_CORE_API auto GetRootSetCount() const -> uint64_t { return rootSets.size(); }
		SH_CORE_API auto GetUpdateTick() const -> uint32_t { return updatePeriodTick; }
		SH_CORE_API auto GetCurrentTick() const -> uint32_t { return tick; }
		/// @brief 이전에 GC를 수행하는데 걸린 시간(ms)을 반환 하는 함수
		SH_CORE_API auto GetElapsedTime() -> uint32_t { return elapseTime; }

		template<typename T, typename IsSObject>
		void AddPointerTracking(SObjWeakPtr<T, IsSObject>& ptr)
		{
			std::lock_guard<std::mutex> lock{ mu };
			auto it = trackingWeakPtrIdxs.find(&ptr);
			if (it != trackingWeakPtrIdxs.end())
				return;
			trackingWeakPtrIdxs.insert({ &ptr, trackingWeakPtrs.size() });
			trackingWeakPtrs.push_back(&ptr);
		}
		template<typename T, typename IsSObject>
		void RemovePointerTracking(SObjWeakPtr<T, IsSObject>& ptr)
		{
			std::lock_guard<std::mutex> lock{ mu };
			auto it = trackingWeakPtrIdxs.find(&ptr);
			if (it == trackingWeakPtrIdxs.end())
				return;

			const std::size_t idx = it->second;
			trackingWeakPtrIdxs.erase(it);

			if (idx != trackingWeakPtrs.size() - 1)
			{
				trackingWeakPtrIdxs[trackingWeakPtrs.back()] = idx;
				trackingWeakPtrs[idx] = trackingWeakPtrs.back();
			}
			trackingWeakPtrs.pop_back();
		}
		/// @brief GC의 마킹 대상에 추가 한다. pendingKill상태일 시 nullptr로 바꾼다.
		/// @tparam SObjectT SObject 타입
		/// @param sobjPtr SObject 포인터의 참조
		template<typename SObjectT, typename Check = std::enable_if_t<std::is_base_of_v<SObject, SObjectT>>>
		void PushReferenceObject(SObjectT*& sobjPtr)
		{
			if (sobjPtr == nullptr)
				return;

			if (sobjPtr->IsPendingKill())
			{
				sobjPtr = nullptr;
				return;
			}

			if constexpr (std::is_const_v<SObjectT>)
				refObjs.push_back(const_cast<std::remove_const_t<SObjectT>&>(*sobjPtr));
			else
				refObjs.push_back(*sobjPtr);
		}
	private:
		SH_CORE_API GarbageCollection();

		void CollectReferenceObjs();
		void Mark(std::size_t start, std::size_t end);
		void MarkProperties(SObject* obj, std::queue<SObject*>& bfs);
		void MarkWithMultiThread();
		void ContainerMark(std::queue<SObject*>& bfs, SObject* parent, int depth, int maxDepth, sh::core::reflection::PropertyIterator<false>& it);
		void CheckPtrs();
	public:
		static constexpr int DEFRAGMENT_ROOTSET_CAP = 32;
	private:
		std::vector<SObject*>& objs;
		std::unordered_map<SObject*, std::size_t> rootSetIdx;
		std::vector<SObject*> rootSets;
		std::vector<SObject*> pendingKillObjs;
		std::unordered_map<GCObject*, std::size_t> gcObjIdx;
		std::vector<std::reference_wrapper<GCObject>> gcObjs;
		std::vector<std::reference_wrapper<SObject>> refObjs;
		std::unordered_map<void*, std::size_t> trackingWeakPtrIdxs;
		std::vector<void*> trackingWeakPtrs;

		std::mutex mu;

		uint32_t elapseTime = 0;
		uint32_t tick = 0;
		uint32_t updatePeriodTick = 1000;

		bool bPendingKill = false;
	};
}//namespace