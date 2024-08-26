#pragma once

#include "Export.h"

#include "Core/Singleton.hpp"
#include "Core/SContainer.hpp"

#include <thread>
#include <atomic>
#include <functional>

namespace sh::window
{
	class Window;
}
namespace sh::game
{
	class World;

	/// @brief 싱글톤 게임 스레드 클래스. Init()으로 초기화 해야 작동한다.
	class GameThread : public core::Singleton<GameThread>
	{
	private:
		friend core::Singleton<GameThread>;

		window::Window* win;
		World* world;

		std::thread thr;
		std::atomic_bool finish;
		core::LockFreeQueue<std::function<void()>> task;

		bool stop;
	protected:
		SH_GAME_API GameThread();
	public:
		/// @brief 게임 스레드를 생성한다.
		/// @param win 윈도우 창
		/// @param world 월드
		SH_GAME_API void Init(window::Window& win, World& world);

		/// @brief 스레드를 중지한다.
		SH_GAME_API void Stop();

		/// @brief 스레드를 반환 하는 함수.
		/// @return std::thread 객체
		SH_GAME_API auto GetThread() -> std::thread&;

		/// @brief [원자적] 한 프레임이 끝났는지 반환한다.
		/// @return 끝났으면 true, 아니면 false
		SH_GAME_API auto IsTaskFinished() const -> bool;

		/// @brief [원자적] 작업 큐에 추가하여 월드가 업데이트 되기전에 수행한다.
		/// @param func 수행 할 함수
		SH_GAME_API void AddTaskQueue(const std::function<void()>& func);
	};
}//namespace