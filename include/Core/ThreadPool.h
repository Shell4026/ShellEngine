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
		/// @brief 해당 함수로 작업을 추가하면 동기화 타이밍에 해당 작업이 끝날 때까지 기다리지 않게 한다.
		template<typename F, typename... Args>
		auto AddContinousTask(F&& func, Args&&... args) -> std::future<typename std::invoke_result_t<F, Args...>>;
	protected:
		SH_CORE_API ThreadPool();
	private:
		template<typename F, typename... Args>
		auto AddTaskImpl(F&& func, bool bContinous, Args&&... args) -> std::future<typename std::invoke_result_t<F, Args...>>;
	private:
		std::mutex mu;

		std::condition_variable cvTask;
		std::condition_variable cvDone;

		std::vector<std::thread> threads;

		struct Task
		{
			std::function<void()> fn;
			bool bContinous = false;
		};
		std::queue<Task> tasks;

		int counter = 0;
		bool bStop = false;
	public:
		constexpr static uint32_t MAX_THREAD = 16;
	};

	template<typename F, typename... Args>
	auto ThreadPool::AddTask(F&& func, Args&&... args) -> std::future<typename std::invoke_result_t<F, Args...>>
	{
		return AddTaskImpl(std::forward<F>(func), false, std::forward<Args>(args)...);
	}
	template<typename F, typename... Args>
	auto ThreadPool::AddContinousTask(F&& func, Args&&... args) -> std::future<typename std::invoke_result_t<F, Args...>>
	{
		return AddTaskImpl(std::forward<F>(func), true, std::forward<Args>(args)...);
	}
	template<typename F, typename... Args>
	auto ThreadPool::AddTaskImpl(F&& func, bool bContinous, Args&&... args) -> std::future<typename std::invoke_result_t<F, Args...>>
	{
		using ReturnType = typename std::invoke_result_t<F, Args...>;
		using PackageType = std::packaged_task<ReturnType()>;

		auto packagePtr = new PackageType(std::bind(std::forward<F>(func), std::forward<Args>(args)...));
		auto result = packagePtr->get_future();
		{
			std::lock_guard<std::mutex> lock{ mu };
			Task task{};
			task.fn =
				[packagePtr]()
				{
					std::unique_ptr<PackageType> uniquePackage{ packagePtr };
					(*packagePtr)();
				};
			task.bContinous = bContinous;

			tasks.push(std::move(task));
		}

		cvTask.notify_one();

		return result;
	}
}//namespace