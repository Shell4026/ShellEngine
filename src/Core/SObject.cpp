#include "SObject.h"

#include "GC.h"
#include <iostream>

namespace sh::core
{
	SObject::SObject(GC* gc) :
		gc(gc), bPendingKill(false), isHeap(_heapCheck == 'h' ? true : false)
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
			if (isHeap)
				gc->DeleteObject(this);
			else
				gc->DeleteObject(this, false);
		}
		else
		{
			if (isHeap)
				free(this);
		}
	}

	auto SObject::operator new(std::size_t size) -> void*
	{
		SObject* obj = reinterpret_cast<SObject*>(::operator new(size));
		obj->_heapCheck = 'h';
		return obj;
	}

	void SObject::operator delete(void* ptr, size_t size) noexcept
	{
		SObject* sobj = static_cast<SObject*>(ptr);
		sobj->bPendingKill = true;
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
}