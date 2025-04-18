#pragma once
#include "Singleton.hpp"

#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>
#include <functional>
#include <future>
#include <memory>
namespace sh::core
{
	class ThreadPool : public Singleton<ThreadPool>
	{
		friend Singleton<ThreadPool>;
	private:
		std::mutex mu;
		std::condition_variable cv;
		std::vector<std::thread> threads;
		std::queue<std::function<void()>> tasks;

		bool stop = false;
	public:
		constexpr static uint32_t MAX_THREAD = 16;
	protected:
		SH_CORE_API ThreadPool();
	public:
		SH_CORE_API ~ThreadPool();

		SH_CORE_API void Init(uint32_t threadNum);

		template<typename F, typename... Args>
		auto AddTask(F&& func, Args&&... args) -> std::future<typename std::invoke_result_t<F, Args...>>
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

			cv.notify_one();

			return result;
		}

		SH_CORE_API auto GetThreads() const -> const std::vector<std::thread>&;
		SH_CORE_API auto GetThreadNum() const -> uint32_t;
	};
}//namespace