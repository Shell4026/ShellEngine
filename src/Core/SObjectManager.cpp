#include "SObjectManager.h"
#include "SObject.h"

namespace sh::core
{
	SH_CORE_API SObjectManager::SObjectManager()
	{
	}
	SH_CORE_API SObjectManager::~SObjectManager()
	{
	}

	SH_CORE_API void SObjectManager::RegisterSObject(SObject* obj)
	{
		objs.insert({ obj->GetUUID(), obj });
	}
	SH_CORE_API void SObjectManager::UnRegisterSObject(SObject* obj)
	{
		objs.erase(obj->GetUUID());
	}
	SH_CORE_API auto SObjectManager::GetSObject(const UUID& uuid) -> SObject*
	{
		auto it = objs.find(uuid);
		if (it == objs.end())
			return nullptr;
		return it->second;
	}
}//namespace