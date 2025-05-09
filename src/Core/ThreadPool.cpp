﻿#include "ThreadPool.h"

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

		cv.notify_all();
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
							
							while(!bStop && tasks.empty())
								cv.wait(lock);
							if (bStop && tasks.empty())
								return;
							task = std::move(tasks.front());
							tasks.pop();
						}

						task();
					}
				}
			);
		}
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