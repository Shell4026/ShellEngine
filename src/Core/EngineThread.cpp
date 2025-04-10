﻿#include "PCH.h"
#include "EngineThread.h"

namespace sh::core
{
	SH_CORE_API EngineThread::EngineThread(bool bSleepThread) :
		mutex(mu),
		mu(), cv(nullptr),
		bStop(false),
		bSleep(bSleepThread),
		beginTasks(), endTasks()
	{
	}

	SH_CORE_API void EngineThread::Run()
	{
		thr = std::thread{ [&] { Update(); } };
	}
	SH_CORE_API auto EngineThread::GetThread() -> std::thread&
	{
		return thr;
	}

	void EngineThread::Update()
	{
		while (!bStop.load(std::memory_order::memory_order_relaxed)) // 원자적이기만 하면 되므로 relaxed
		{
			std::unique_lock<std::mutex> lock{ mu };

			taskMutex.lock();
			while (!beginTasks.empty())
			{
				beginTasks.front()();
				beginTasks.pop();
			}
			taskMutex.unlock();

			for (auto& task : tasks)
				task();

			taskMutex.lock();
			while (!endTasks.empty())
			{
				endTasks.front()();
				endTasks.pop();
			}
			taskMutex.unlock();

			if (cv != nullptr)
			{
				bSleep = true;
				cv->wait(lock, [&] { return !bSleep; }); // wait이 되는 순간 lock은 풀린다. <~ 동기화에 활용
			}
		}
	}

	SH_CORE_API void EngineThread::AddTask(const std::function<void()>& task)
	{
		tasks.push_back(task);
	}

	SH_CORE_API void EngineThread::AddBeginTaskFromOtherThread(const std::function<void()>& task)
	{
		taskMutex.lock();
		beginTasks.push(task);
		taskMutex.unlock();
	}
	SH_CORE_API void EngineThread::AddEndTaskFromOtherThread(const std::function<void()>& task)
	{
		taskMutex.lock();
		endTasks.push(task);
		taskMutex.unlock();
	}

	SH_CORE_API void EngineThread::Stop()
	{
		bStop.store(true, std::memory_order::memory_order_relaxed); // 원자적이기만 하면 되므로 relaxed
		if (cv)
		{
			mu.lock();
			bSleep = false;
			mu.unlock();

			cv->notify_one();
		}
	}

	SH_CORE_API bool EngineThread::Awake()
	{
		if (!this->cv)
			return false;
		if (mu.try_lock()) // 잠금을 획득 할 수 있다 = 스레드가 자고 있다.
		{
			bSleep = false;
			mu.unlock();
			cv->notify_one();
			return true;
		}
		return false;
	}

	SH_CORE_API void EngineThread::SetWaitableThread(bool wait)
	{
		if (wait)
		{
			if (cv == nullptr)
				cv = std::make_unique<std::condition_variable>();
		}
		else
		{
			if (cv)
				cv.reset();
		}	
	}

	SH_CORE_API auto EngineThread::GetThreadID() const -> std::thread::id
	{
		return thr.get_id();
	}
}//namespace