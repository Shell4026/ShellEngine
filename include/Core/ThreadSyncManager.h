#pragma once

#include "ISyncable.h"
#include "EngineThread.h"
#include "SContainer.hpp"

#include "queue"
namespace sh::core
{
	class ThreadSyncManager
	{
	private:
		std::queue<ISyncable*> syncables;
		SVector<EngineThread*> threads;
	public:
		SH_CORE_API ThreadSyncManager() = default;
		SH_CORE_API ~ThreadSyncManager() = default;

		SH_CORE_API void PushSyncable(ISyncable& syncable);
		SH_CORE_API void AddThread(EngineThread& thread);
		SH_CORE_API void Sync();
		SH_CORE_API void AwakeThread();
	};
}//namespace