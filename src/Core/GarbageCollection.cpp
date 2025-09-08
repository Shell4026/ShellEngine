#include "GarbageCollection.h"
#include "SObjectManager.h"
#include "ThreadPool.h"
#include "SContainer.hpp"

#include "SObject.h"

#include <queue>

namespace sh::core
{
	void GarbageCollection::ContainerMark(std::queue<SObject*>& bfs, SObject* parent, int depth, int maxDepth, sh::core::reflection::PropertyIteratorT& it)
	{
		if (depth == maxDepth)
		{
			SObject* const* propertyPtr = nullptr;
			if (it.IsPair())
				propertyPtr = it.GetPairSecond<SObject*>();
			else
				propertyPtr = it.Get<SObject*>();

			SObject* ptr = *propertyPtr;
			if (ptr == nullptr)
			{
				++it;
				return;
			}

			if (ptr->bPendingKill.load(std::memory_order::memory_order_acquire))
				it.Erase(); // iterator 자동 갱신
			else
			{
				bfs.push(ptr);
				++it;
			}
			return;
		}
		for (auto itSide = it.GetNestedBegin(); itSide != it.GetNestedEnd();)
		{
			ContainerMark(bfs, parent, depth + 1, maxDepth, itSide);
		}
		++it;
	}
	void GarbageCollection::Mark(std::size_t start, std::size_t end)
	{
		std::queue<SObject*> bfs{};
		for (std::size_t i = start; i < end; ++i)
		{
			if (rootSets[i] != nullptr)
				bfs.push(rootSets[i]);

			while (!bfs.empty())
			{
				SObject* obj = bfs.front();
				bfs.pop();

				if (obj == nullptr)
					continue;
				if (obj->bMark.test_and_set(std::memory_order::memory_order_acq_rel))
					continue;

				MarkProperties(obj, bfs);
			}
		}
	}
	void GarbageCollection::MarkWithMultiThread()
	{
		const auto threadPool = core::ThreadPool::GetInstance();
		const int threadNum = threadPool->GetThreadNum();
		const std::size_t perThreadTaskCount = (rootSets.size() + threadNum - 1) / threadNum;

		std::array<std::future<void>, ThreadPool::MAX_THREAD> taskFutures;
		std::size_t p = 0;
		int futureIdx = 0;
		while (p < rootSets.size())
		{
			const size_t start = p;
			const size_t end = std::min(p + perThreadTaskCount, rootSets.size());
			taskFutures[futureIdx++] = (threadPool->AddTask(
				[&, start, end]
				{
					Mark(start, end);
				}
			));
			p = end;
		}
		for (int i = 0; i < futureIdx; ++i)
			taskFutures[i].wait();
	}
	void GarbageCollection::MarkProperties(SObject* obj, std::queue<SObject*>& bfs)
	{
		const reflection::STypeInfo* type = &obj->GetType();
		while (type != nullptr)
		{
			auto& ptrProps = type->GetSObjectPtrProperties();
			for (auto ptrProp : ptrProps)
			{
				SObject** propertyPtr = ptrProp->Get<SObject*>(*obj);
				SObject* ptr = *propertyPtr;
				if (ptr == nullptr)
					continue;

				// Destory함수로 인해 제거 될 객체면 가르키고 있던 포인터를 nullptr로 바꾸고, 마킹하지 않는다.
				if (ptr->bPendingKill.load(std::memory_order::memory_order_acquire))
					*propertyPtr = nullptr;
				else
					bfs.push(ptr);
			}
			auto& ptrContainerProps = type->GetSObjectPtrContainerProperties();
			for (auto ptrProp : ptrContainerProps)
			{
				int nested = ptrProp->GetContainerNestedLevel();
				for (auto it = ptrProp->Begin(*obj); it != ptrProp->End(*obj);)
				{
					if (nested == 1)
					{
						SObject* const* propertyPtr = nullptr;
						if (it.IsPair())
							propertyPtr = it.GetPairSecond<SObject*>();
						else
							propertyPtr = it.Get<SObject*>();

						SObject* ptr = *propertyPtr;
						if (ptr == nullptr)
						{
							++it;
							continue;
						}

						if (ptr->bPendingKill.load(std::memory_order::memory_order_acquire))
							it.Erase(); // iterator 자동 갱신
						else
						{
							bfs.push(ptr);
							++it;
						}
					}
					else
					{
						ContainerMark(bfs, obj, 1, nested, it);
					}
				}
			}
			type = type->GetSuper(); // 부모 클래스의 프로퍼티들도 검사
		}
	}
	void GarbageCollection::MarkContainers(TrackingContainerIt start, TrackingContainerIt end)
	{
		for (auto it = start; it != end; ++it)
		{
			auto& [containerPtr, info] = *it;
			switch (info.type)
			{
			case TrackingContainerInfo::Type::Array:
			{
				SObject** arrPtr = reinterpret_cast<SObject**>(containerPtr);
				for (std::size_t i = 0; i < std::get<0>(info.data); ++i)
				{
					SObject** obj = (arrPtr + i);
					if (*obj == nullptr)
						continue;

					if ((*obj)->bPendingKill.load(std::memory_order::memory_order_acquire))
					{
						*obj = nullptr;
						continue;
					}
					std::queue<SObject*> bfs{};
					bfs.push(*obj);
					while (!bfs.empty())
					{
						SObject* obj = bfs.front();
						bfs.pop();

						if (obj == nullptr)
							continue;
						if (obj->bMark.test_and_set(std::memory_order::memory_order_acq_rel))
							continue;

						MarkProperties(obj, bfs);
					}
				}
				break;
			}
			case TrackingContainerInfo::Type::Vector:
			{
				std::vector<SObject*>& vector = *reinterpret_cast<std::vector<SObject*>*>(containerPtr);
				for (int i = 0; i < vector.size(); ++i)
				{
					SObject* obj = vector[i];
					if (obj == nullptr)
						continue;
					if (obj->bPendingKill.load(std::memory_order::memory_order_acquire))
					{
						vector[i] = nullptr;
						continue;
					}
					std::queue<SObject*> bfs{};
					bfs.push(obj);
					while (!bfs.empty())
					{
						SObject* obj = bfs.front();
						bfs.pop();

						if (obj == nullptr)
							continue;
						if (obj->bMark.test_and_set(std::memory_order::memory_order_acq_rel))
							continue;

						MarkProperties(obj, bfs);
					}
				}
				break;
			}
			case TrackingContainerInfo::Type::List:
			{
				std::list<SObject*>& list = *reinterpret_cast<std::list<SObject*>*>(containerPtr);
				for (auto it = list.begin(); it != list.end();)
				{
					SObject* obj = *it;
					if (obj == nullptr)
					{
						++it;
						continue;
					}
					if (obj->bPendingKill.load(std::memory_order::memory_order_acquire))
					{
						it = list.erase(it);
						continue;
					}
					std::queue<SObject*> bfs{};
					bfs.push(obj);
					while (!bfs.empty())
					{
						SObject* obj = bfs.front();
						bfs.pop();

						if (obj == nullptr)
							continue;
						if (obj->bMark.test_and_set(std::memory_order::memory_order_acq_rel))
							continue;
						MarkProperties(obj, bfs);
					}
					++it;
				}
				break;
			}
			case TrackingContainerInfo::Type::Set:
			{
				std::set<SObject*>& set = *reinterpret_cast<std::set<SObject*>*>(containerPtr);
				for (auto it = set.begin(); it != set.end();)
				{
					SObject* obj = *it;
					if (obj->bPendingKill.load(std::memory_order::memory_order_acquire))
					{
						it = set.erase(it);
						continue;
					}
					std::queue<SObject*> bfs{};
					bfs.push(obj);
					while (!bfs.empty())
					{
						SObject* obj = bfs.front();
						bfs.pop();

						if (obj == nullptr)
							continue;
						if (obj->bMark.test_and_set(std::memory_order::memory_order_acq_rel))
							continue;

						MarkProperties(obj, bfs);
					}
					++it;
				}
				break;
			}
			case TrackingContainerInfo::Type::HashSet:
			{
				std::unordered_set<SObject*>& set = *reinterpret_cast<std::unordered_set<SObject*>*>(containerPtr);
				for (auto it = set.begin(); it != set.end();)
				{
					SObject* obj = *it;
					if (obj->bPendingKill.load(std::memory_order::memory_order_acquire))
					{
						it = set.erase(it);
						continue;
					}
					std::queue<SObject*> bfs{};
					bfs.push(obj);
					while (!bfs.empty())
					{
						SObject* obj = bfs.front();
						bfs.pop();

						if (obj == nullptr)
							continue;
						if (obj->bMark.test_and_set(std::memory_order::memory_order_acq_rel))
							continue;

						MarkProperties(obj, bfs);
					}
					++it;
				}
				break;
			}
			case TrackingContainerInfo::Type::MapKey: [[fallthrough]];
			case TrackingContainerInfo::Type::MapValue: [[fallthrough]];
			case TrackingContainerInfo::Type::HashMapKey: [[fallthrough]];
			case TrackingContainerInfo::Type::HashMapValue:
			{
				ICheckable* checkable = reinterpret_cast<ICheckable*>(&std::get<1>(info.data));
				checkable->Checking(*this);
				break;
			}
			}
		}
	}
	void GarbageCollection::MarkContainersWithMultiThread()
	{
		const auto threadPool = core::ThreadPool::GetInstance();
		const int threadNum = threadPool->GetThreadNum();
		const std::size_t perThreadTaskCount = (trackingContainers.size() + threadNum - 1) / threadNum;

		auto it = trackingContainers.begin();

		std::array<std::future<void>, ThreadPool::MAX_THREAD> taskFutures;
		int futureIdx = 0;
		while (it != trackingContainers.end())
		{
			auto startIt = it;
			for (std::size_t i = 0; i < perThreadTaskCount && it != trackingContainers.end(); ++i)
				++it;
			auto endIt = it;
			taskFutures[futureIdx++] = (threadPool->AddTask(
				[&, startIt, endIt]
				{
					MarkContainers(startIt, endIt);
				}
			));
		}
		for (int i = 0; i < futureIdx; ++i)
			taskFutures[i].wait();
	}
	void GarbageCollection::CheckPtrs()
	{
		for (auto ptr : trackingPtrs)
		{
			SObjWeakPtr<SObject>& weakPtr = *reinterpret_cast<SObjWeakPtr<SObject>*>(ptr);
			if (weakPtr == nullptr)
				continue;
			if (weakPtr->bPendingKill.load(std::memory_order::memory_order_acquire))
				weakPtr.Reset();
		}
	}
	GarbageCollection::GarbageCollection() :
		objs(SObjectManager::GetInstance()->objs)
	{
	}

	GarbageCollection::~GarbageCollection()
	{
		Update();
	}

	SH_CORE_API void GarbageCollection::SetRootSet(SObject* obj)
	{
		std::lock_guard<std::mutex> lock{ mu };

		auto it = rootSetIdx.find(obj);
		if (it == rootSetIdx.end())
		{
			std::size_t idx = rootSets.size();
			rootSets.push_back(obj);
			rootSetIdx.insert_or_assign(obj, idx);
		}
	}
	SH_CORE_API auto GarbageCollection::GetRootSet() const -> const std::vector<SObject*>&
	{
		return rootSets;
	}
	SH_CORE_API auto GarbageCollection::GetRootSetCount() const -> uint64_t
	{
		return rootSets.size() - emptyRootSetCount;
	}
	SH_CORE_API void GarbageCollection::SetUpdateTick(uint32_t tick)
	{
		updatePeriodTick = tick;
		this->tick = 0;
	}
	SH_CORE_API auto GarbageCollection::GetUpdateTick() const -> uint32_t
	{
		return updatePeriodTick;
	}
	SH_CORE_API auto GarbageCollection::GetCurrentTick() const -> uint32_t
	{
		return tick;
	}

	SH_CORE_API void GarbageCollection::RemoveRootSet(const SObject* obj)
	{
		std::lock_guard<std::mutex> lock{ mu };

		auto it = rootSetIdx.find(const_cast<SObject*>(obj));
		if (it != rootSetIdx.end())
		{
			std::size_t idx = it->second;
			rootSets[idx] = nullptr;
			rootSetIdx.erase(it);
			++emptyRootSetCount;
		}
	}

	SH_CORE_API void GarbageCollection::DefragmentRootSet()
	{
		std::lock_guard<std::mutex> lock{ mu };

		std::unordered_map<SObject*, std::size_t> tmpIdx;
		std::vector<SObject*> tmp;
		tmp.reserve(rootSets.size() - emptyRootSetCount);
		for (auto rootset : rootSets)
		{
			if (rootset == nullptr)
				continue;

			const std::size_t idx = tmp.size();
			tmp.push_back(rootset);
			tmpIdx.insert_or_assign(rootset, idx);
		}
		emptyRootSetCount = 0;

		rootSetIdx = std::move(tmpIdx);
		rootSets = std::move(tmp);
	}

	SH_CORE_API void GarbageCollection::Update()
	{
		++tick;
		if (updatePeriodTick > 1)
		{
			if (tick == updatePeriodTick - 1)
			{
				if (emptyRootSetCount >= DEFRAGMENT_ROOTSET_CAP)
					DefragmentRootSet();
				return;
			}
			if (tick == updatePeriodTick)
			{
				tick = 0;
				Collect();
				DestroyPendingKillObjs();
				return;
			}
		}
		else
		{
			if (emptyRootSetCount >= DEFRAGMENT_ROOTSET_CAP)
				DefragmentRootSet();
			Collect();
			DestroyPendingKillObjs();
		}
	}
	SH_CORE_API void sh::core::GarbageCollection::Collect()
	{
		auto start = std::chrono::high_resolution_clock::now();
		for (auto& [id, obj] : objs)
			obj->bMark.clear(std::memory_order::memory_order_relaxed); // memory_order_relaxed - 어차피 다른 스레드들은 모두 자고 있음

		const bool bThreadPoolInit = ThreadPool::GetInstance()->IsInit();

		if (trackingContainers.size() > 8 && bThreadPoolInit)
			MarkContainersWithMultiThread();
		else
			MarkContainers(trackingContainers.begin(), trackingContainers.end());

		if (rootSets.size() > 128 && bThreadPoolInit)
			MarkWithMultiThread();
		else
			Mark(0, rootSets.size());

		// 모든 SObject를 순회하며 마킹이 안 됐으면 보류 목록에 추가
		// TODO: 나중에 멀티 스레드로 바꿀 때 메모리 오더 바꾸기
		for (auto& [uuid, objPtr] : objs)
		{
			if (!objPtr->bMark.test_and_set(std::memory_order::memory_order_relaxed))
			{
				if (!objPtr->bPendingKill.load(std::memory_order::memory_order_relaxed))
				{
					objPtr->OnDestroy();
					objPtr->bPendingKill.store(true, std::memory_order::memory_order_relaxed);
				}
			}
		}

		CheckPtrs();
		bPendingKill = true;

		auto end = std::chrono::high_resolution_clock::now();

		//static std::deque<uint64_t> times;
		//if (times.size() < 100)
		//	times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
		//else
		//	times.pop_front();

		//static double mean = 0.0f;
		//for (auto t : times)
		//	mean += t;
		//mean /= 100;

		//SH_INFO_FORMAT("GC Mean: {}", mean);

		elapseTime = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
	}

	SH_CORE_API auto GarbageCollection::GetObjectCount() const -> std::size_t
	{
		return objs.size();
	}

	SH_CORE_API void GarbageCollection::ForceDelete(SObject* obj)
	{
		auto it = objs.find(obj->GetUUID());
		if (it != objs.end())
		{
			RemoveRootSet(obj);
			objs.erase(it);
			delete obj;
		}
	}

	SH_CORE_API void GarbageCollection::AddToPendingKillList(SObject* obj)
	{
		pendingKillObjs.push_back(obj);
	}

	SH_CORE_API void GarbageCollection::DestroyPendingKillObjs()
	{
		if (!bPendingKill)
			return;

		for (auto& objPtr : pendingKillObjs)
		{
			if (!objPtr->bPlacementNew)
				delete objPtr;
			else
				std::destroy_at(objPtr);
			objPtr = nullptr;
		}
		pendingKillObjs.clear();
		bPendingKill = false;

		SH_INFO("Destroy pending objects");
	}

	SH_CORE_API auto sh::core::GarbageCollection::GetElapsedTime() -> uint32_t
	{
		return elapseTime;
	}
}//namespace