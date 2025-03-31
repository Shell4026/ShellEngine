#pragma once
#include "Export.h"
#include "Singleton.hpp"
#include "SContainer.hpp"
#include "UUID.h"

namespace sh::core
{
	class SObject;

	class SObjectManager : public Singleton<SObjectManager>
	{
		friend Singleton<SObjectManager>;
		friend class GarbageCollection;
	private:
		core::SHashMap<UUID, SObject*> objs;

		SH_CORE_API SObjectManager();
	public:
		SH_CORE_API ~SObjectManager();

		SH_CORE_API void RegisterSObject(SObject* obj);
		SH_CORE_API void UnRegisterSObject(SObject* obj);
		SH_CORE_API auto GetSObject(const UUID& uuid) -> SObject*;
	};
}//namespace