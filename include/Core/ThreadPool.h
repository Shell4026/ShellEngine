#pragma once
#include "Singleton.hpp"

#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>
#include <functional>
#include <future>
#include <memory>
#include <atomic>
namespace sh::core
{
	class ThreadPool : public Singleton<ThreadPool>
	{
		friend Singleton<ThreadPool>;
	public:
		SH_CORE_API ~ThreadPool();

		SH_CORE_API void Init(uint32_t threadNum);

		SH_CORE_API void WaitAllTask();

		SH_CORE_API void Lock();
		SH_CORE_API void Unlock();

		SH_CORE_API auto GetThreads() const -> const std::vector<std::thread>&;
		SH_CORE_API auto GetThreadNum() const -> uint32_t;
		SH_CORE_API auto IsInit() const -> bool;

		template<typename F, typename... Args>
		auto AddTask(F&& func, Args&&... args) -> std::future<typename std::invoke_result_t<F, Args...>>;
	protected:
		SH_CORE_API ThreadPool();
	private:
		std::mutex mu;

		std::condition_variable cvTask;
		std::condition_variable cvDone;

		std::vector<std::thread> threads;
		std::queue<std::function<void()>> tasks;

		int counter = 0;
		bool bStop = false;
	public:
		constexpr static uint32_t MAX_THREAD = 16;
	};

	template<typename F, typename... Args>
	auto ThreadPool::AddTask(F&& func, Args&&... args) -> std::future<typename std::invoke_result_t<F, Args...>>
	{
		using ReturnType = typename std::invoke_result_t<F, Args...>;
		using PackageType = std::packaged_task<ReturnType()>;

		auto packagePtr = new PackageType(std::bind(std::forward<F>(func), std::forward<Args>(args)...));
		auto result = packagePtr->get_future();
		{
			std::lock_guard<std::mutex> lock{ mu };
			tasks.push([packagePtr]()
				{
					std::unique_ptr<PackageType> uniquePackage{ packagePtr };
					(*packagePtr)();
				}
			);
		}

		cvTask.notify_one();

		return result;
	}
}//namespace