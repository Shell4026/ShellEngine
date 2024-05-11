#include "GC.h"

#include "SObject.h"

namespace sh::core
{
	GC::~GC()
	{
		Update();
	}

	void GC::AddObject(SObject* obj)
	{
		objs.insert(obj);
	}

	auto GC::RemoveObject(SObject* obj) -> bool
	{
		return objs.erase(obj) > 0;
	}


	void GC::DFSIteratorCheckPtr(int depth, int maxDepth, sh::core::reflection::PropertyIterator& it)
	{
		if (depth == maxDepth)
		{
			SObject** ptr;
			if(!it.IsPair())
				ptr = it.Get<SObject*>();
			else
				ptr = it.GetPairSecond<SObject*>();

			for (auto delObj : deletedObjs)
			{
				if (*ptr == delObj)
					*ptr = nullptr;
			}
			return;
		}
		for (auto itSide = it.GetNestedBegin(); itSide != it.GetNestedEnd(); ++itSide)
		{
			DFSIteratorCheckPtr(depth + 1, maxDepth, itSide);
		}
	}

	void GC::DeleteObject(SObject* obj)
	{
		if(RemoveObject(obj))
			deletedObjs.push_back(obj);
	}

	void GC::Update()
	{
		if (deletedObjs.size() == 0)
			return;

		//O(NM)
		for (auto obj : objs)
		{
			auto& props = obj->GetType().GetSObjectPtrProperties();
			for (auto prop : props)
			{
				auto ptr = prop->Get<SObject*>(obj);
				for (auto delObj : deletedObjs)
				{
					if (*ptr == delObj)
						*ptr = nullptr;
				}
			}

			auto& containers = obj->GetType().GetSObjectContainerProperties();
			for (auto prop : containers)
			{
				for (auto it = prop->Begin(obj); it != prop->End(obj); ++it)
				{
					DFSIteratorCheckPtr(1, prop->containerNestedLevel, it);
				}
			}
		}
		for (auto objs : deletedObjs)
			free(objs);
		deletedObjs.clear();
	}
}