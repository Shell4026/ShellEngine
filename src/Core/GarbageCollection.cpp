#include "GarbageCollection.h"

#include "SObject.h"

#include <queue>

namespace sh::core
{
	GarbageCollection::GarbageCollection()
	{
	}
	GarbageCollection::~GarbageCollection()
	{
		Update();
	}

	void GarbageCollection::AddObject(SObject* obj)
	{
		objs.insert(obj);
	}

	void GarbageCollection::SetRootSet(SObject* obj)
	{
		rootSets.insert(obj);
	}

	void GarbageCollection::RemoveRootSet(SObject* obj)
	{
		rootSets.erase(obj);
	}

	auto GarbageCollection::RemoveObject(SObject* obj) -> bool
	{
		RemoveRootSet(obj);
		return objs.erase(obj) == 1; //지워진 원소 수
	}

	void GarbageCollection::DFSIteratorCheckPtr(SObject* target, int depth, int maxDepth, sh::core::reflection::PropertyIterator& it)
	{
		if (depth == maxDepth)
		{
			SObject** ptr;
			if(!it.IsPair())
				ptr = it.Get<SObject*>();
			else
				ptr = it.GetPairSecond<SObject*>();

			if (*ptr == target)
				*ptr = nullptr;
			return;
		}
		for (auto itSide = it.GetNestedBegin(); itSide != it.GetNestedEnd(); ++itSide)
		{
			DFSIteratorCheckPtr(target, depth + 1, maxDepth, itSide);
		}
	}
	void GarbageCollection::ContainerMark(SObject* parent, int depth, int maxDepth, sh::core::reflection::PropertyIterator& it)
	{
		if (depth == maxDepth)
		{
			SObject** ptr;
			if (!it.IsPair())
				ptr = it.Get<SObject*>();
			else
				ptr = it.GetPairSecond<SObject*>();

			Mark(*ptr, parent);
			return;
		}
		for (auto itSide = it.GetNestedBegin(); itSide != it.GetNestedEnd(); ++itSide)
		{
			ContainerMark(parent, depth + 1, maxDepth, itSide);
		}
	}

	void GarbageCollection::Update()
	{
		for (SObject* obj : objs)
		{
			obj->bMark = false;
		}

		// TODO 병렬 처리?
		for (SObject* root : rootSets)
		{
			Mark(root, nullptr);
		}

		// 모든 SObject를 순회하며 마킹이 안 됐으면 제거
		std::queue<SObject*> deleted;
		for (auto it = objs.begin(); it != objs.end();)
		{
			auto ptr = *it;
			if (!ptr->IsMark())
			{
				it = objs.erase(it);
				ptr->bPendingKill.store(true, std::memory_order::memory_order_release);
				ptr->OnDestroy();
				deleted.push(ptr);
			}
			else
				++it;
		}

		while (!deleted.empty())
		{
			SObject* ptr = deleted.front();
			deleted.pop();

			delete ptr;
		}
	}

	void GarbageCollection::Mark(SObject* obj, SObject* parent)
	{
		if (obj == nullptr || obj->bMark)
			return;
		//제거 한 객체(Remove())인 경우 obj를 참조하고 있는 포인터를 nullptr로 바꾼다.
		if (obj->bPendingKill.load(std::memory_order::memory_order_acquire))
		{
			if (parent == nullptr)
				return;

			const reflection::TypeInfo* type = &parent->GetType();
			while (type)
			{
				for (auto& ptrProp : type->GetSObjectPtrProperties())
				{
					SObject** ptr = ptrProp->Get<SObject*>(parent);
					if (*ptr == obj)
						*ptr = nullptr;
				}
				auto& containers = type->GetSObjectContainerProperties();
				for (auto prop : containers)
				{
					for (auto it = prop->Begin(parent); it != prop->End(parent); ++it)
					{
						DFSIteratorCheckPtr(obj, 1, prop->containerNestedLevel, it);
					}
				}
				type = type->GetSuper(); // 부모 클래스도 검사
			}
			return;
		}

		obj->bMark = true;

		const reflection::TypeInfo* type = &obj->GetType();
		while (type)
		{
			auto& ptrProps = type->GetSObjectPtrProperties();
			for (auto& ptrProp : ptrProps)
			{
				SObject** ptr = ptrProp->Get<SObject*>(obj);
				if (*ptr == nullptr)
					continue;

				Mark(*ptr, obj);
			}
			auto& containerPtrProps = type->GetSObjectContainerProperties();
			for (auto ptrProp : containerPtrProps)
			{
				for (auto it = ptrProp->Begin(obj); it != ptrProp->End(obj); ++it)
				{
					ContainerMark(obj, 1, ptrProp->containerNestedLevel, it);
				}
			}
			type = type->GetSuper(); // 부모 클래스도 검사
		}
	}

	auto GarbageCollection::GetObjectCount() const -> std::size_t
	{
		return objs.size();
	}
}//namespace