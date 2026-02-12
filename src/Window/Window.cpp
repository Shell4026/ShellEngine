#include "Window.h"

#include "WindowFactory.h"

#include <iostream>
#include <thread>

namespace sh::window {
	Window::Window() :
		width(wsize), height(hsize),

		isOpen(false),
		fps(60), deltaTime(0.0f),
		wsize(0), hsize(0)
	{
	}

	Window::~Window()
	{
		this->Close();
	}

	void Window::Create(const std::string& title, int wsize, int hsize, StyleFlag style)
	{
		if (isOpen)
			return;

		std::cout << "Init\n";
		this->title = title;

		winImpl = WindowFactory::CreateWindowImpl();
		handle = winImpl->Create(title, wsize, hsize, style);

		this->wsize = wsize;
		this->hsize = hsize;

		isOpen = true;
	}

	bool Window::PollEvent(Event& event)
	{
		if (winImpl.get() == nullptr)
			return false;

		if (winImpl->IsEmptyEvent())
		{
			winImpl->ProcessEvent();
		}
		else
		{
			event = winImpl->PopEvent();
			if (event.type == Event::EventType::Resize)
			{
				wsize = GetWidth();
				hsize = GetHeight();
			}
			return true;
		}
		return false;
	}

	bool Window::IsOpen() const
	{
		return isOpen;
	}

	void Window::Close()
	{
		if (winImpl.get() == nullptr)
			return;

		isOpen = false;
		winImpl.reset();
	}

	void Window::SetFps(unsigned int fps)
	{
		this->fps = fps;
	}

	auto Window::GetDeltaTime() const -> float
	{
		return deltaTime;
	}

	void Window::ProcessFrame()
	{
		using Clock = std::chrono::steady_clock;
		static bool bInit = false;
		static Clock::time_point last;
		static Clock::time_point next;

		const auto now0 = Clock::now();
		if (!bInit)
		{
			last = now0;
			next = now0;
			bInit = true;
		}
		const auto target = std::chrono::microseconds(1'000'000 / fps); // 144fps = 6944us
		next += target;

		if (now0 < next)
		{
			const auto remainUs = std::chrono::duration_cast<std::chrono::microseconds>(next - now0).count();

			// 2ms보다 크게 남은 경우 sleep하고 나머지는 busy wait으로 맞춤
			if (remainUs > 2000) // 2ms
			{
				const uint32_t ms = (uint32_t)((remainUs - 1000) / 1000); // 마지막 1ms는 남겨둠
				if (ms > 0)
				{
					if (bUsingSysTimer) 
						winImpl->StopTimer(ms);
					else 
						std::this_thread::sleep_for(std::chrono::milliseconds(ms));
				}
			}

			while (Clock::now() < next)
				std::this_thread::yield();
		}
		else
		{
			next = Clock::now();
		}

		const auto now1 = Clock::now();
		deltaTime = std::chrono::duration<double>(now1 - last).count();
		last = now1;
	}

	auto Window::GetNativeHandle() const -> WinHandle
	{
		return handle;
	}

	void Window::SetTitle(std::string_view title)
	{
		winImpl->SetTitle(title);
	}

	SH_WINDOW_API void Window::SetSize(int width, int height)
	{
		winImpl->Resize(width, height);
	}

	SH_WINDOW_API auto Window::GetWidth() const -> uint32_t
	{
		return winImpl->GetWidth();
	}
	SH_WINDOW_API auto Window::GetHeight() const -> uint32_t
	{
		return winImpl->GetHeight();
	}
	SH_WINDOW_API void Window::UseSystemTimer(bool bUse)
	{
		bUsingSysTimer = bUse;
	}
	SH_WINDOW_API auto Window::IsUseingSystemTimer() const -> bool
	{
		return bUsingSysTimer;
	}
}
