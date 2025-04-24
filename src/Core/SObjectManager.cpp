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
		std::unique_lock<std::shared_mutex> lock{ mu };

		objs.insert({ obj->GetUUID(), obj });
	}
	SH_CORE_API void SObjectManager::UnRegisterSObject(const SObject* obj)
	{
		std::unique_lock<std::shared_mutex> lock{ mu };

		objs.erase(obj->GetUUID());
	}
	SH_CORE_API auto SObjectManager::GetSObject(const UUID& uuid) -> SObject*
	{
		std::shared_lock<std::shared_mutex> lock{ mu };

		auto it = objs.find(uuid);
		if (it == objs.end())
			return nullptr;
		return it->second;
	}
}//namespace