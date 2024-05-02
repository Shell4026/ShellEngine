#include "SObject.h"

namespace sh::core
{
	auto SObject::IsPendingKill() const -> bool
	{
		return isPendingKill;
	}

	bool IsValid(SObject* obj)
	{
		return obj;
	}

	void SObject::UpdateRef()
	{
		auto& props = GetType().GetPointerProperties();
		for (int i = 0; i < props.size(); ++i)
		{
			auto ptr = props[i]->Get<SObject*>(this);
			if (ptr == nullptr)
				return;

			if (ptr->refThis.find(this) == ptr->refThis.end())
			{
				ptr->refThis.insert({ this, i });
			}
		}
	}

	void SObject::Destroy()
	{
		for (auto& other : refThis)
		{
			auto& pointers = other.first->GetType().GetPointerProperties();
			pointers[other.second]->Set<SObject*>(other.first, nullptr);
		}
	}
}