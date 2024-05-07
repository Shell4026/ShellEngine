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

		//O(NM)
		for (auto obj : objs)
		{
			auto& props = obj->GetType().GetSObjectPtrProperties();
			for (auto prop : props)
			{
				auto ptr = prop->Get<SObject*>(obj);
				if (*ptr == _obj)
					*ptr = nullptr;
			}

		}
	}
}