#include "GameThread.h"
#include "World.h"
#include "Input.h"
#include "ImGUI.h"

#include "Core/GarbageCollection.h"

#include "Window/Window.h"

#include <fmt/core.h>

namespace sh::game
{
	GameThread::GameThread() :
		win(nullptr), world(nullptr), gc(*core::GarbageCollection::GetInstance()),
		thr(),
		finish(false), syncFin(false),
		stop(false)
	{

	}

	void GameThread::Init(window::Window& win, World& world)
	{
		this->win = &win;
		this->world = &world;

		this->thr = std::thread
		(
			[&]
			{
				fmt::print("[Start] GameThread\n");
				while (!stop)
				{
					this->win->ProcessFrame();
					finish.store(false, std::memory_order::memory_order_release);

					std::function<void()> func;
					while (task.Dequeue(func))
					{
						func();
					}
					this->world->Update(this->win->GetDeltaTime());

					finish.store(true, std::memory_order::memory_order_release);

					std::unique_lock<std::mutex> lock{ mu };
					while (!syncFin && !stop)
						cv.wait(lock, [&] {return syncFin || stop; });
					syncFin = false;
				}
				fmt::print("[Stop] GameThread\n");
			}
		);
	}

	void GameThread::Stop()
	{
		stop = true;
		cv.notify_all();
	}

	auto GameThread::GetThread() -> std::thread&
	{
		return thr;
	}

	auto GameThread::IsTaskFinished() const -> bool
	{
		return finish.load(std::memory_order::memory_order_acquire);
	}

	void GameThread::AddTaskQueue(const std::function<void()>& func)
	{
		task.Enqueue(func);
	}

	void GameThread::SyncFinished()
	{
		mu.lock();
		syncFin = true;
		gc.Update();
		mu.unlock();

		cv.notify_all();
	}
}
