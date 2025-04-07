#include "GarbageCollection.h"
#include "SObjectManager.h"

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
		rootSets.insert(obj);
	}

	SH_CORE_API void GarbageCollection::RemoveRootSet(SObject* obj)
	{
		rootSets.erase(obj);
	}

	SH_CORE_API void GarbageCollection::Update()
	{
		auto start = std::chrono::high_resolution_clock::now();
		for (auto &[id, obj] : objs)
		{
			obj->bMark = false;
		}

		// TODO 병렬 처리?
		std::queue<SObject*> bfs{};
		for (SObject* root : rootSets)
		{
			bfs.push(root);
		}

		while (!bfs.empty())
		{
			SObject* obj = bfs.front();
			bfs.pop();

			if (obj == nullptr || obj->bMark)
				continue;

			obj->bMark = true;

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
							SObject*const* propertyPtr = nullptr;
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

		// 모든 SObject를 순회하며 마킹이 안 됐으면 제거
		std::queue<SObject*> deleted;
		for (auto it = objs.begin(); it != objs.end(); ++it)
		{
			SObject* ptr = it->second;
			if (!ptr->IsMark())
			{
				ptr->bPendingKill.store(true, std::memory_order::memory_order_release);
				ptr->OnDestroy();
				deleted.push(ptr);
			}
		}

		while (!deleted.empty())
		{
			SObject* ptr = deleted.front();
			deleted.pop();

			delete ptr;
		}
		auto end = std::chrono::high_resolution_clock::now();
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