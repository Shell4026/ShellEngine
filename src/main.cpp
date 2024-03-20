#include "Engine.h"
#include "Window/Window.h"

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
			}
		}
	}
	return 0;
}