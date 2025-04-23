#include "GarbageCollection.h"
#include "SObjectManager.h"
#include "ThreadPool.h"

#include "SObject.h"

#include <queue>

namespace sh::core
{
	void GarbageCollection::ContainerMark(std::queue<SObject*>& bfs, SObject* parent, int depth, int maxDepth, sh::core::reflection::PropertyIterator& it)
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
				if (obj->bMark.test_and_set(std::memory_order::memory_order_acquire))
					continue;

				const reflection::STypeInfo* type = &obj->GetType();
				while (type != nullptr)
				{
					auto& ptrProps = type->GetSObjectPtrProperties();
					for (auto ptrProp : ptrProps)
					{
						SObject** propertyPtr = ptrProp->Get<SObject*>(obj);
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
						for (auto it = ptrProp->Begin(obj); it != ptrProp->End(obj);)
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
		}
	}
	void GarbageCollection::MarkMultiThread()
	{
		const auto threadPool = core::ThreadPool::GetInstance();
		const int threadNum = threadPool->GetThreadNum();
		const std::size_t perThreadTaskCount = rootSets.size() / threadNum + (rootSets.size() % threadNum > 0 ? 1 : 0);

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

	void GarbageCollection::CheckVectors()
	{
		for (auto v : trackingVectors)
		{
			std::vector<SObject*>& vector = *reinterpret_cast<std::vector<SObject*>*>(v);
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
				SetRootSet(obj);
			}
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
	SH_CORE_API void GarbageCollection::SetUpdateTick(uint32_t tick)
	{
		updatePeriodTick = tick;
		this->tick = 0;
	}

	SH_CORE_API void GarbageCollection::RemoveRootSet(const SObject* obj)
	{
		auto it = rootSetIdx.find(const_cast<SObject*>(obj));
		if (it != rootSetIdx.end())
		{
			std::size_t idx = it->second;
			rootSets[idx] = nullptr;
			rootSetIdx.erase(it);
		}
	}

	SH_CORE_API void GarbageCollection::Update()
	{
		if (++tick < updatePeriodTick)
			return;
		tick = 0;

		Collect();
	}
	SH_CORE_API void sh::core::GarbageCollection::Collect()
	{
		auto start = std::chrono::high_resolution_clock::now();
		for (auto& [id, obj] : objs)
			obj->bMark.clear(std::memory_order::memory_order_relaxed);

		CheckVectors();

		if (rootSets.size() > 50)
			MarkMultiThread();
		else
			Mark(0, rootSets.size());

		for (auto v : trackingVectors)
		{
			std::vector<SObject*>& vector = *reinterpret_cast<std::vector<SObject*>*>(v);
			for (SObject* obj : vector)
			{
				if (obj == nullptr)
					continue;
				RemoveRootSet(obj);
			}
		}

		// 모든 SObject를 순회하며 마킹이 안 됐으면 제거
		std::queue<SObject*> deleted;
		for (auto& [uuid, objPtr] : objs)
		{
			if (!objPtr->bMark.test_and_set(std::memory_order::memory_order_relaxed))
			{
				objPtr->OnDestroy();
				deleted.push(objPtr);
				objPtr->bPendingKill.store(true, std::memory_order::memory_order_release);
			}
		}

		while (!deleted.empty())
		{
			SObject* ptr = deleted.front();
			deleted.pop();

			delete ptr;
		}
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

	SH_CORE_API auto sh::core::GarbageCollection::GetElapsedTime() -> uint32_t
	{
		return elapseTime;
	}
}//namespace