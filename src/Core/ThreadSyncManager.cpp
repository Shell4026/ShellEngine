#include "ThreadSyncManager.h"
#include "ThreadPool.h"

#include <algorithm>
namespace sh::core
{
	std::vector<ThreadSyncManager::ThreadData> ThreadSyncManager::threads{};
	thread_local int ThreadSyncManager::currentThreadIdx = -1;
	std::priority_queue<ThreadSyncManager::ThreadData::SyncData> ThreadSyncManager::syncables{};
	bool ThreadSyncManager::bOnSync = false;

	SH_CORE_API auto ThreadSyncManager::GetThreadIndex() -> int
	{
		auto it = std::find_if(threads.begin(), threads.end(), [&](const ThreadData& data) {return data.threadID == std::this_thread::get_id(); });
		assert(it != threads.end()); // 현재 코드를 실행중인 스레드는 EngineThread 객체가 존재하지 않는다.
		if (it == threads.end())
			return -1;
		return std::distance(threads.begin(), it);
	}
	SH_CORE_API void ThreadSyncManager::Init()
	{
		currentThreadIdx = 0;
		threads.push_back(ThreadData{ nullptr, std::this_thread::get_id() });

		for (auto& thread : ThreadPool::GetInstance()->GetThreads())
		{
			threads.push_back(ThreadData{ nullptr, thread.get_id() });
		}
	}
	SH_CORE_API void ThreadSyncManager::PushSyncable(ISyncable& syncable, uint32_t priority)
	{
		if (!bOnSync)
		{
			if (currentThreadIdx == -1)
				currentThreadIdx = GetThreadIndex();
			threads[currentThreadIdx].syncableQueue.push({ &syncable, priority });
		}
		else
		{
			syncables.push({ &syncable, priority });
		}
	}

	SH_CORE_API void ThreadSyncManager::AddThread(EngineThread& thread)
	{
		auto it = std::find_if(threads.begin(), threads.end(), [&](const ThreadData& data) {return data.threadPtr == &thread; });
		if (it != threads.end()) // 중복 확인
			return;

		threads.push_back(ThreadData{ &thread, thread.GetThreadID()});
		if (thread.GetThreadID() == std::this_thread::get_id())
			currentThreadIdx = threads.size() - 1;
	}

	SH_CORE_API void ThreadSyncManager::Sync()
	{
		//SH_INFO("Start sync");
		for (int i = 0; i < threads.size(); ++i)
		{
			if (threads[i].threadPtr == nullptr)
				continue;
			threads[i].threadPtr->mutex.lock(); // 자고 있는 상태면 잠금을 획득 할 수 있다. 아니면 대기
		}

		for (auto& threadData : threads)
		{
			while (!threadData.syncableQueue.empty())
			{
				syncables.push(threadData.syncableQueue.front());
				threadData.syncableQueue.pop();
			}
		}

		bOnSync = true;
		while (!syncables.empty())
		{
			auto [syncable, priority] = syncables.top();
			syncables.pop();
			if (syncable)
				syncable->Sync();
		}
		bOnSync = false;

		for (int i = threads.size() - 1; i >= 0; --i)
		{
			if (threads[i].threadPtr == nullptr)
				continue;
			threads[i].threadPtr->mutex.unlock();
		}
	}

	SH_CORE_API void ThreadSyncManager::AwakeThread()
	{
		for (auto& thr : threads)
		{
			if (!thr.threadPtr)
				continue;
			thr.threadPtr->Awake();
		}
	}

	SH_CORE_API auto ThreadSyncManager::IsMainThread() -> bool
	{
		return threads[0].threadID == std::this_thread::get_id();
	}
}//namespace