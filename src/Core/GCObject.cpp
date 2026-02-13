#include "GCObject.h"
#include "GarbageCollection.h"

namespace sh::core
{
	GCObject::GCObject()
	{
		static GarbageCollection& gc = *GarbageCollection::GetInstance();
		gc.AddGCObject(*this);
	}
	GCObject::~GCObject()
	{
		static GarbageCollection& gc = *GarbageCollection::GetInstance();
		gc.RemoveGCObject(*this);
	}
}//namespace