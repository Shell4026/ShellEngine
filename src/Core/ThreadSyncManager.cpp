#include "PCH.h"
#include "ThreadSyncManager.h"

namespace sh::core
{
	void ThreadSyncManager::PushSyncable(ISyncable& syncable)
	{
		syncables.push(&syncable);
	}

	void ThreadSyncManager::AddThread(EngineThread& thread)
	{
		auto it = std::find(threads.begin(), threads.end(), &thread);
		if (it != threads.end()) // 중복 확인
			return;

		threads.push_back(&thread);
	}

	void ThreadSyncManager::Sync()
	{
		//SH_INFO("Start sync");
		for (int i = 0; i < threads.size(); ++i)
			threads[i]->mutex.lock(); // 자고 있는 상태면 잠금을 획득 할 수 있다. 아니면 대기

		while (!syncables.empty())
		{
			core::ISyncable* syncable = syncables.front();
			syncables.pop();

			if (syncable)
				syncable->Sync();
		}

		for (int i = threads.size() - 1; i >= 0; --i)
			threads[i]->mutex.unlock();
	}

	void ThreadSyncManager::AwakeThread()
	{
		for (auto& thr : threads)
			thr->Awake();
	}
}//namespace