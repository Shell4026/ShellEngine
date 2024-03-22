#include "Window/Window.h"

#include <iostream>
int main(int arg, char** args[]) {
	sh::Window window;
	window.Create(u8"테스트", 1024, 768);
	
	while (window.IsOpen())
	{
		sh::Event e;
		while (window.PollEvent(e))
		{
			switch (e.type)
			{
			case sh::Event::EventType::Close:
				window.Close();
				break;
			case sh::Event::EventType::MousePressed:
				if (e.mouseType == sh::Event::MouseType::Left)
				{
					std::cout << "Left\n";
				}
				else if (e.mouseType == sh::Event::MouseType::Right)
				{
					std::cout << "Right\n";
				}
				else
				{
					std::cout << "Middle\n";
				}
				break;
			}
		}
	}
	return 0;
}