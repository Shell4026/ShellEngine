#include "Engine.h"
#include "Window/Window.h"

int main(int arg, char args[]) {
	fmt::print("Hello, World{}!", 3);

	sh::Window window;
	window.Init();
	return 0;
}