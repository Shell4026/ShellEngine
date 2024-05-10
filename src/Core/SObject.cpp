#include "SObject.h"

#include "GC.h"

namespace sh::core
{
	SObject::SObject(GC* gc) :
		gc(gc), bPendingKill(false)
	{
		if (gc != nullptr)
		{
			gc->AddObject(this);
		}
	}

	SObject::~SObject()
	{
		if (gc != nullptr)
		{
			gc->DeleteObject(this);
		}
	}

	void SObject::operator delete(void* ptr, size_t size) noexcept
	{
		SObject* sobj = static_cast<SObject*>(ptr);
		if (sobj->gc == nullptr)
		{
			free(ptr);
			return;
		}
		sobj->bPendingKill = true;
	}

	void SObject::SetGC(GC& gc)
	{
		this->gc = &gc;
		gc.AddObject(this);
	}

	auto SObject::IsPendingKill() const -> bool
	{
		return bPendingKill;
	}
}