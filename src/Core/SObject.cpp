#include "SObject.h"

#include "GC.h"

namespace sh::core
{
	SObject::SObject(GC* gc) :
		gc(gc)
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

	void SObject::SetGC(GC& gc)
	{
		this->gc = &gc;
		gc.AddObject(this);
	}
}