#include "Window.h"

#include "WindowFactory.h"

#include <iostream>
#include <thread>
namespace sh::window {
	Window::Window() :
		width(wsize), height(hsize),

		isOpen(false),
		fps(60), maxFrameMs(1000.0f / static_cast<float>(fps)), deltaTime(0.0f),
		wsize(0), hsize(0)
	{
		startTime = std::chrono::high_resolution_clock::now();
		endTime = startTime;
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
		maxFrameMs = 1000.0f / static_cast<float>(fps);
	}

	auto Window::GetDeltaTime() const -> float
	{
		return deltaTime;
	}

	void Window::ProcessFrame()
	{
		startTime = std::chrono::high_resolution_clock::now();

		auto frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(startTime - endTime);
		uint64_t deltaTimeMicro = frameDuration.count();

		uint64_t freeTimeMicro = maxFrameMs * 1000.f - deltaTimeMicro;
		if (freeTimeMicro > 0.f)
		{
			std::this_thread::sleep_for(std::chrono::microseconds{ freeTimeMicro });
		}
		endTime = std::chrono::high_resolution_clock::now();

		frameDuration += std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
		deltaTime = frameDuration.count() / 1000'000.f;
	}

	auto Window::GetNativeHandle() const -> WinHandle
	{
		return handle;
	}

	void Window::SetTitle(std::string_view title)
	{
		winImpl->SetTitle(title);
	}

	auto Window::GetWidth() const -> uint32_t
	{
		return winImpl->GetWidth();
	}
	auto Window::GetHeight() const -> uint32_t
	{
		return winImpl->GetHeight();
	}
}
