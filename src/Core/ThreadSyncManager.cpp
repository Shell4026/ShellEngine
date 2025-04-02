#include "ThreadSyncManager.h"

#include <algorithm>
namespace sh::core
{
	SVector<ThreadSyncManager::ThreadData> ThreadSyncManager::threads{};
	thread_local int ThreadSyncManager::currentThreadIdx = -1;

	SH_CORE_API auto ThreadSyncManager::GetThreadIndex() -> int
	{
		auto it = std::find_if(threads.begin(), threads.end(), [&](const ThreadData& data) {return data.threadPtr && data.threadPtr->GetThreadID() == std::this_thread::get_id(); });
		assert(it != threads.end()); // 현재 코드를 실행중인 스레드는 EngineThread 객체가 존재하지 않는다.
		if (it == threads.end())
			return -1;
		return std::distance(threads.begin(), it);
	}
	SH_CORE_API void ThreadSyncManager::Init()
	{
		currentThreadIdx = 0;
		threads.push_back(ThreadData{ nullptr });
	}
	SH_CORE_API void ThreadSyncManager::PushSyncable(ISyncable& syncable)
	{
		if (currentThreadIdx == -1)
			currentThreadIdx = GetThreadIndex();
		threads[currentThreadIdx].syncableQueue.push(&syncable);
	}

	SH_CORE_API void ThreadSyncManager::AddThread(EngineThread& thread)
	{
		auto it = std::find_if(threads.begin(), threads.end(), [&](const ThreadData& data) {return data.threadPtr == &thread; });
		if (it != threads.end()) // 중복 확인
			return;

		threads.push_back(ThreadData{ &thread });
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

		std::vector<ISyncable*> syncables;
		for (auto& threadData : threads)
		{
			while (!threadData.syncableQueue.empty())
			{
				syncables.push_back(threadData.syncableQueue.front());
				threadData.syncableQueue.pop();
			}
		}

		for (ISyncable* syncable : syncables)
		{
			if (syncable)
				syncable->Sync();
		}

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
}//namespace