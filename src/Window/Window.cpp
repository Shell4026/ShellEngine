﻿#include "Window.h"

#include "WindowFactory.h"
#include "WindowImpl.h"

#include <iostream>
namespace sh {
	Window::Window() :
		isOpen(false)
	{

	}

	Window::~Window()
	{
		this->Close();
	}

	void Window::Create(const std::string& title, int wsize, int hsize)
	{
		std::cout << "Init\n";

		this->title = title;

		winImpl = WindowFactory::CreateWindowImpl();
		winImpl->Create(title, wsize, hsize);

		isOpen = true;
	}

	bool Window::PollEvent(Event& event)
	{
		if (winImpl.get() == nullptr)
			return false;
		winImpl->ProcessEvent();

		if (!winImpl->IsEmptyEvent())
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
}
