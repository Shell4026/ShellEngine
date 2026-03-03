#pragma once
#include "Export.h"

namespace sh::core
{
	class GarbageCollection;
	class GCObject
	{
	public:
		SH_CORE_API GCObject();
		SH_CORE_API GCObject(const GCObject& other);
		SH_CORE_API GCObject(GCObject&& other);

		SH_CORE_API auto operator=(const GCObject& other) -> GCObject & = default;
		SH_CORE_API auto operator=(GCObject&& other) -> GCObject& = default;

		SH_CORE_API virtual ~GCObject();
		SH_CORE_API virtual void PushReferenceObjects(GarbageCollection& gc) = 0;
	};
}//namespace