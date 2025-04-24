#pragma once
#include "Export.h"
#include "Singleton.hpp"
#include "UUID.h"

#include <shared_mutex>
namespace sh::core
{
	class SObject;

	/// @brief 모든 SObject는 여기서 관리된다. 스레드 안전하다.
	class SObjectManager : public Singleton<SObjectManager>
	{
		friend Singleton<SObjectManager>;
		friend class GarbageCollection;
	private:
		std::unordered_map<UUID, SObject*> objs;
		std::shared_mutex mu;
	private:
		SH_CORE_API SObjectManager();
	public:
		SH_CORE_API ~SObjectManager();

		SH_CORE_API void RegisterSObject(SObject* obj);
		SH_CORE_API void UnRegisterSObject(const SObject* obj);
		SH_CORE_API auto GetSObject(const UUID& uuid) -> SObject*;
	};
}//namespace