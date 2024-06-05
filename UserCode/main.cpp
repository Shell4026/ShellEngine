#include "Export.h"

#include "ComponentTest.h"

#include "Game/ComponentModule.h"

#include <iostream>

static sh::game::ComponentModule module;

extern "C"
{
	SH_USER_API void Init()
	{
		std::cout << "Hello, User Module!!\n";
		module.RegisterComponent<ComponentTest>("ComponentTest");
	}
	SH_USER_API auto GetModule() -> sh::game::ComponentModule*
	{
		return &module;
	}
}