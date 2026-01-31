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

		auto it = objIdxMap.find(obj->GetUUID());
		if (it != objIdxMap.end())
			return;

		objIdxMap[obj->GetUUID()] = objs.size();
		objs.push_back(obj);
		objPtrs.insert(obj);
	}
	SH_CORE_API void SObjectManager::UnRegisterSObject(const SObject* obj)
	{
		std::unique_lock<std::shared_mutex> lock{ mu };
		auto it = objIdxMap.find(obj->GetUUID());
		if (it == objIdxMap.end())
			return;

		const std::size_t idx = it->second;
		const std::size_t last = objs.size() - 1;

		objIdxMap.erase(it);
		objPtrs.erase(obj);

		if (idx != last)
		{
			SObject* moved = objs[last];
			objs[idx] = moved;
			objIdxMap[moved->GetUUID()] = idx;
		}
		objs.pop_back();
	}
	SH_CORE_API auto SObjectManager::GetSObject(const UUID& uuid) -> SObject*
	{
		std::shared_lock<std::shared_mutex> lock{ mu };

		auto it = objIdxMap.find(uuid);
		if (it == objIdxMap.end())
			return nullptr;
		return objs[it->second];
	}
	SH_CORE_API auto SObjectManager::IsSObject(void* ptr) -> bool
	{
		std::shared_lock<std::shared_mutex> lock{ mu };
		auto it = objPtrs.find(reinterpret_cast<SObject*>(ptr));
		if (it == objPtrs.end())
			return false;
		return true;
	}
}//namespace