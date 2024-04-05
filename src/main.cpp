#include <iostream>
#include <chrono>
#include <thread>

#include "Window/Window.h"
#include "Render/VulkanRenderer.h"
#include "Core/Reflaction.hpp"

#include <Core/Util.h>
#include <cassert>
class Base {
	SCLASS(Base)
public:

public:
	void BaseFunction() 
	{ 
		std::cout << "Base!!\n"; 
	}
};

class Derived : public Base
{
	SCLASS(Derived)
public:
	void DerivedFunction() 
	{ 
		std::cout << "Derived!!\n"; 
	}
};

class NoBase {
public:
	void NoBaseFunction()
	{
		std::cout << "NoBase!!\n";
	}
};

int main(int arg, char* args[]) {
	
	Base a;
	Derived b;
	NoBase c;
	Base* p = &b;

	auto real = sh::core::Util::Cast<Derived>(p);
	assert(real != nullptr);
	p->BaseFunction();
	real->DerivedFunction();

	sh::window::Window window;
	window.Create(u8"테스트", 1024, 768);
	auto renderer = sh::render::VulkanRenderer::GetInstance();
	renderer->Init(window);
	
	constexpr long long fps = static_cast<long long>(1000.0f / 144.0f);
	
	auto start = std::chrono::high_resolution_clock::now();
	auto end = std::chrono::high_resolution_clock::now();
	long long delta_time = 0;
	while (window.IsOpen())
	{
		start = std::chrono::high_resolution_clock::now();
		delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		std::this_thread::sleep_for(std::chrono::milliseconds(fps - delta_time));
		end = std::chrono::high_resolution_clock::now();
		delta_time += std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		//std::cout << delta_time << "ms\n";
		//fmt::print("{}ms\n", delta_time);

		sh::window::Event e;
		while (window.PollEvent(e))
		{
			
			switch (e.type)
			{
			case sh::window::Event::EventType::Close:
				window.Close();
				break;
			case sh::window::Event::EventType::MousePressed:
				if (e.mouseType == sh::window::Event::MouseType::Left)
				{
					std::cout << "Left\n";
				}
				else if (e.mouseType == sh::window::Event::MouseType::Right)
				{
					std::cout << "Right\n";
				}
				else
				{
					std::cout << "Middle\n";
				}
				break;
			case sh::window::Event::EventType::MouseWheelScrolled:
				std::cout << sh::window::Event::MouseWheelScrolled::delta << '\n';
				break;
			case sh::window::Event::EventType::KeyDown:
				if (e.keyType == sh::window::Event::KeyType::Left)
					std::cout << "Check\n";
				break;
			case sh::window::Event::EventType::WindowFocus:
				std::cout << "FocusIn\n";
				break;
			case sh::window::Event::EventType::WindowFocusOut:
				std::cout << "FocusOut\n";
				break;
			}
		}
	}
	return 0;
}