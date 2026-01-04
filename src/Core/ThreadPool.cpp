#include "ThreadPool.h"

namespace sh::core
{
	ThreadPool::ThreadPool()
	{
	}
	ThreadPool::~ThreadPool()
	{
		mu.lock();
		bStop = true;
		mu.unlock();

		cvTask.notify_all();

		for (auto& thread : threads)
			thread.join();
	}
	SH_CORE_API void ThreadPool::Init(uint32_t threadNum)
	{
		for (int i = 0; i < threadNum; ++i)
		{
			threads.emplace_back([&]
				{
					while (true)
					{
						std::function<void()> task;
						{
							std::unique_lock<std::mutex> lock{ mu };
							
							cvTask.wait(lock, [this]() {return bStop || !tasks.empty(); });

							if (bStop && tasks.empty())
								return;

							task = std::move(tasks.front());
							tasks.pop();

							++counter;
						}

						task();

						{
							std::unique_lock<std::mutex> lock{ mu };
							--counter;
							if (tasks.empty() && counter == 0)
								cvDone.notify_all();
						}
					}
				}
			);
		}
	}

	SH_CORE_API void ThreadPool::WaitAllTask()
	{
		std::unique_lock<std::mutex> lock{ mu };
		cvDone.wait(lock, [this] {return tasks.empty() && counter == 0; });
	}
	SH_CORE_API void ThreadPool::Lock()
	{
		mu.lock();
	}
	SH_CORE_API void ThreadPool::Unlock()
	{
		mu.unlock();
	}

	SH_CORE_API auto ThreadPool::GetThreads() const -> const std::vector<std::thread>&
	{
		return threads;
	}
	SH_CORE_API auto ThreadPool::GetThreadNum() const -> uint32_t
	{
		return threads.size();
	}
	SH_CORE_API auto ThreadPool::IsInit() const -> bool
	{
		return threads.size() > 1;
	}
}//namepsace