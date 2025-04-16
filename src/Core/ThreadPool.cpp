#include "ThreadPool.h"

namespace sh::core
{
	ThreadPool::ThreadPool()
	{
	}
	ThreadPool::~ThreadPool()
	{
		mu.lock();
		stop = true;
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
							
							while(!stop && tasks.empty())
								cv.wait(lock);
							if (stop && tasks.empty())
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
}//namepsace