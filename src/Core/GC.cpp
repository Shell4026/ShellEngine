#include "GC.h"

#include "SObject.h"

namespace sh::core
{
	void GC::AddObject(SObject* obj)
	{
		objs.insert(obj);
	}

	void GC::RemoveObject(SObject* obj)
	{
		objs.erase(obj);
	}

	void GC::DeleteObject(SObject* _obj)
	{
		RemoveObject(_obj);

		for (auto obj : objs)
		{
			auto& props = obj->GetType().GetSObjectPtrProperties();
			for (auto prop : props)
			{
				if (prop->Get<SObject*>(obj) == _obj)
				{
					prop->Set<SObject*>(obj, nullptr);
				}
			}
		}
	}
}