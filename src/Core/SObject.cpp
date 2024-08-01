#include "SObject.h"

#include "GC.h"
#include "TrackingAllocator.hpp"

#include <cstring>

namespace sh::core
{
	SObject::SObject(GC* gc) :
		gc(gc), bPendingKill(false)
	{
		if (gc != nullptr)
		{
			gc->AddObject(this);
		}

		isHeap = TrackingAllocator<SObject>::GetInstance()->IsHeapAllocated(this);
	}

	SObject::~SObject()
	{
		if (gc != nullptr)
		{
			if (isHeap)
				gc->DeleteObject(this);
			else
				gc->DeleteObject(this, false);
		}
	}

	auto SObject::operator new(std::size_t size) -> void*
	{
		SObject* obj = TrackingAllocator<SObject>::GetInstance()->Allocate(size);
		//SObject* obj = static_cast<SObject*>(::operator new(size));
		return obj;
	}

	void SObject::operator delete(void* ptr) noexcept
	{
		SObject* sobj = static_cast<SObject*>(ptr);
		sobj->bPendingKill = true;
		if (sobj->gc == nullptr)
			TrackingAllocator<SObject>::GetInstance()->DeAllocate(static_cast<SObject*>(ptr));
			//::operator delete(ptr, size);
		//free는 gc에서 수행
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
	void SObject::OnPropertyChanged(const reflection::Property& prop)
	{
	}
}