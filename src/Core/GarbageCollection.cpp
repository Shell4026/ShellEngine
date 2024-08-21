#include "GarbageCollection.h"

#include "SObject.h"

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

	void GarbageCollection::DeleteObject(SObject* obj)
	{
		RemoveObject(obj);
		deletedObjs.insert(obj);
	}

	void GarbageCollection::Update()
	{
		for (SObject* obj : objs)
		{
			obj->bMark = false;
		}

		//TODO 병렬 처리?
		for (SObject* root : rootSets)
		{
			Mark(root, nullptr);
		}

		for (auto it = objs.begin(); it != objs.end();)
		{
			auto ptr = *it;
			if (!ptr->IsMark())
			{
				it = objs.erase(it);
				ptr->bPendingKill = true;
				delete ptr;
			}
			else
				++it;
		}
		deletedObjs.clear();
	}

	void GarbageCollection::Mark(SObject* obj, SObject* parent)
	{
		if (obj == nullptr || obj->bMark)
			return;
		//제거 한 객체(Remove())인 경우 obj를 참조하고 있는 포인터를 nullptr로 바꾼다.
		if (obj->bPendingKill)
		{
			if (parent == nullptr)
				return;

			auto& ptrProps = parent->GetType().GetSObjectPtrProperties();
			for (auto& ptrProp : ptrProps)
			{
				SObject** ptr = ptrProp->Get<SObject*>(parent);
				if (*ptr == obj)
					*ptr = nullptr;
			}
			auto& containers = parent->GetType().GetSObjectContainerProperties();
			for (auto prop : containers)
			{
				for (auto it = prop->Begin(parent); it != prop->End(parent); ++it)
				{
					DFSIteratorCheckPtr(obj, 1, prop->containerNestedLevel, it);
				}
			}
			return;
		}

		obj->bMark = true;

		auto& ptrProps = obj->GetType().GetSObjectPtrProperties();
		for (auto& ptrProp : ptrProps)
		{
			SObject** ptr = ptrProp->Get<SObject*>(obj);
			//delete된 객체인 경우 참조를 nullptr로 바꾼다.
			if (deletedObjs.find(*ptr) != deletedObjs.end())
			{
				*ptr = nullptr;
				continue;
			}

			Mark(*ptr, obj);
		}
		auto& containerPtrProps = obj->GetType().GetSObjectContainerProperties();
		for (auto& ptrProp : containerPtrProps)
		{
			for (auto it = ptrProp->Begin(obj); it != ptrProp->End(obj); ++it)
			{
				SObject** ptr = it.Get<SObject*>();
				//delete된 객체인 경우 참조를 nullptr로 바꾼다.
				if (deletedObjs.find(*ptr) != deletedObjs.end())
				{
					*ptr = nullptr;
					continue;
				}

				Mark(*ptr, obj);
			}
		}
	}

	auto GarbageCollection::GetObjectCount() const -> std::size_t
	{
		return objs.size();
	}
}//namespace