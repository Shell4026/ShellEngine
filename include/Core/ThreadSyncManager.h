#pragma once
#include "ISyncable.h"
#include "EngineThread.h"

#include <queue>
#include <thread>
#include <vector>
namespace sh::core
{
	/// @brief 스레드간 동기화를 관리하는 전역 객체.
	class ThreadSyncManager
	{
	public:
		struct ThreadData
		{
			EngineThread* threadPtr;
			std::thread::id threadID;
			struct SyncData
			{
				ISyncable* ptr;
				uint32_t priority = 0;

				auto operator<(const SyncData& other) const -> bool
				{
					return priority > other.priority;
				}
			};
			std::queue<SyncData> syncableQueue;
		};
	public:
		/// @brief 초기화 함수. 반드시 메인 스레드에서 호출 할 것.
		SH_CORE_API static void Init();
		SH_CORE_API static void Clear();
		SH_CORE_API static void PushSyncable(ISyncable& syncable, uint32_t priority = 0);
		SH_CORE_API static void AddThread(EngineThread& thread);
		/// @brief 동기화 객체들을 동기화 하는 함수. 모든 스레드가 작업을 마치고 수면중일 때 호출 해야한다.
		SH_CORE_API static void Sync();
		SH_CORE_API static void AwakeThread();
		/// @brief 이 함수를 실행 하는 스레드가 메인 스레드인지
		/// @return 맞으면 true, 아니면 false
		SH_CORE_API static auto IsMainThread() -> bool;

		SH_CORE_API static auto GetSyncableCount() -> uint32_t;
		SH_CORE_API static auto GetCurrentThreadData() -> ThreadData&;
	private:
		SH_CORE_API static auto GetThreadIndex() -> int;
	private:
		SH_CORE_API static std::vector<ThreadData> threads;
		thread_local static int currentThreadIdx;
		SH_CORE_API static std::priority_queue<ThreadData::SyncData> syncables;
		SH_CORE_API static bool bOnSync;
	};
}//namespace