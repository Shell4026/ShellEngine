#pragma once
#include "ISyncable.h"
#include "EngineThread.h"
#include "SContainer.hpp"

#include <queue>
namespace sh::core
{
	/// @brief 스레드간 동기화를 관리하는 전역 객체.
	class ThreadSyncManager
	{
	private:
		struct ThreadData
		{
			EngineThread* threadPtr;
			std::queue<ISyncable*> syncableQueue;
		};
		SH_CORE_API static SVector<ThreadData> threads;
		thread_local static int currentThreadIdx;
	private:
		SH_CORE_API static auto GetThreadIndex() -> int;
	public:
		/// @brief 초기화 함수. 반드시 메인 스레드에서 호출 할 것.
		SH_CORE_API static void Init();
		SH_CORE_API static void PushSyncable(ISyncable& syncable);
		SH_CORE_API static void AddThread(EngineThread& thread);
		/// @brief 동기화 객체들을 동기화 하는 함수. 모든 스레드가 작업을 마치고 수면중일 때 호출 해야한다.
		SH_CORE_API static void Sync();
		SH_CORE_API static void AwakeThread();
	};
}//namespace