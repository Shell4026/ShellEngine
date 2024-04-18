#include "Window.h"

#include "WindowFactory.h"

#include <iostream>
#include <thread>
namespace sh::window {
	Window::Window() :
		isOpen(false),
		fps(60), maxFrameMs(1000.0f / static_cast<float>(fps)), deltaTimeMs(0), deltaTime(0.0f)
	{
		startTime = std::chrono::high_resolution_clock::now();
		endTime = startTime;
	}

	Window::~Window()
	{
		this->Close();
	}

	void Window::Create(const std::string& title, int wsize, int hsize)
	{
		if (isOpen)
			return;

		std::cout << "Init\n";
		this->title = title;

		winImpl = WindowFactory::CreateWindowImpl();
		handle = winImpl->Create(title, wsize, hsize);

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
		deltaTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(startTime - endTime).count();
		if (static_cast<int>(maxFrameMs - deltaTimeMs) > 0)
			std::this_thread::sleep_for(std::chrono::milliseconds{ maxFrameMs - deltaTimeMs });
		endTime = std::chrono::high_resolution_clock::now();
		deltaTimeMs += std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

		deltaTime = deltaTimeMs / 1000.0f;
	}

	auto Window::GetNativeHandle() const -> WinHandle
	{
		return handle;
	}
}
