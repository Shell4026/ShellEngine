#include "EngineThread.h"

#include "Logger.h"

#include <deque>
namespace sh::core
{
	SH_CORE_API EngineThread::EngineThread() :
		mutex(),
		beginTasks(), endTasks(),
		bStop(false),
		bSleep(false),
		bRun(false)
	{
		thr = std::thread{ [&] { Update(); } };
	}

	SH_CORE_API void EngineThread::Run()
	{
		if (!bRun)
		{
			bRun = true;
			cv.notify_one();
		}
	}
	SH_CORE_API auto EngineThread::GetThread() -> std::thread&
	{
		return thr;
	}

	void EngineThread::Update()
	{
		// Run전에는 Sleep
		{
			std::unique_lock<std::mutex> lock{ mutex };
			while(!bRun)
				cv.wait(lock);
		}
		while (!bStop.load(std::memory_order::memory_order_relaxed)) // 원자적이기만 하면 되므로 relaxed
		{
			static double mean = 0;
			static std::deque<uint64_t> us;
			//auto start = std::chrono::high_resolution_clock::now();
			std::unique_lock<std::mutex> lock{ mutex };

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

			//auto end = std::chrono::high_resolution_clock::now();
			//if (us.size() < 100)
			//	us.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
			//else
			//	us.pop_front();

			//uint64_t sum = 0;
			//for (auto t : us)
			//	sum += t;
			//mean = sum / 100.0;

			//SH_INFO_FORMAT("mean {}us", mean);
			bSleep = true;
			while (bSleep)
				cv.wait(lock); // wait이 되는 순간 lock은 풀린다. <~ 동기화에 활용
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
		mutex.lock();
		bSleep = false;
		mutex.unlock();

		cv.notify_one();
	}

	SH_CORE_API bool EngineThread::Awake()
	{
		if (mutex.try_lock()) // 잠금을 획득 할 수 있다 = 스레드가 자고 있다.
		{
			bSleep = false;
			mutex.unlock();
			cv.notify_one();
			return true;
		}
		return false;
	}

	SH_CORE_API auto EngineThread::GetThreadID() const -> std::thread::id
	{
		return thr.get_id();
	}
}//namespace