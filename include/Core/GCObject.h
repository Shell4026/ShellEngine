#pragma once
#include "Export.h"

namespace sh::core
{
	class GarbageCollection;
	class GCObject
	{
	public:
		SH_CORE_API GCObject();
		SH_CORE_API virtual ~GCObject();
		SH_CORE_API virtual void PushReferenceObjects(GarbageCollection& gc) {};
	};
}//namespace