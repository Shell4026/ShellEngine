#pragma once

#include "Export.h"
#include "SContainer.hpp"

#include <mutex>
#include <thread>
#include <functional>
#include <atomic>
#include <condition_variable>
#include <memory>

namespace sh::core
{
	/// @brief 엔진에서 돌아갈 스레드를 나타내는 클래스.
	class EngineThread
	{
	private:
		std::thread thr;
		std::mutex mu;

		SVector<std::function<void()>> tasks;
		core::LockFreeQueue<std::function<void()>> beginTasks;
		core::LockFreeQueue<std::function<void()>> endTasks;

		std::unique_ptr<std::condition_variable> cv;

		std::atomic<bool> bStop;
		bool bSleep;
	public:
		std::mutex& mutex;
	private:
		void Update();
	protected:
		/// @brief 매 루프마다 실행될 작업을 추가하는 함수.
		SH_CORE_API void AddTask(const std::function<void()>& task);
	public:
		/// @brief 생성자
		/// @param bSleepThread 매 루프가 끝날 시 Sleep상태가 되는지 
		SH_CORE_API EngineThread(bool bSleepThread = false);
		SH_CORE_API virtual ~EngineThread() = default;
		
		/// @brief 스레드를 반환 하는 함수.
		/// @return std::thread 객체
		SH_CORE_API auto GetThread() -> std::thread&;
		/// @brief [원자적] 작업 큐에 작업을 추가하는 함수. 매 업데이트 전 실행된다.
		/// @param func 수행 할 함수
		SH_CORE_API void AddBeginTaskFromOtherThread(const std::function<void()>& task);
		/// @brief [원자적] 작업 큐에 작업을 추가하는 함수. 매 업데이트 후 실행된다.
		/// @param func 수행 할 함수
		SH_CORE_API void AddEndTaskFromOtherThread(const std::function<void()>& task);
		/// @brief [원자적] 루프를 중단 시키고 스레드를 끝내는 함수.
		SH_CORE_API void Stop();
		/// @brief 스레드가 자고 있으면 깨운다. (조건 변수 사용)
		/// @return 성공 시 true, 아닐 시 false
		SH_CORE_API bool Awake();
		/// @brief 매 루프마다 Sleep 상태로 만들 것인지 정하는 함수.
		/// @param wait true 또는 false
		SH_CORE_API void SetWaitableThread(bool wait);
	};
}//namespace