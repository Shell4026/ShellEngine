#include "Export.h"

#include "ComponentTest.h"
#include "RotateObject.h"

#include "Game/ComponentModule.h"

#include <iostream>

static sh::game::ComponentModule module;

extern "C"
{
	SH_USER_API void Init()
	{
		std::cout << "Init User module.\n";

		module.RegisterComponent<ComponentTest>("ComponentTest");
		module.RegisterComponent<RotateObject>("RotateObject");
	}
	SH_USER_API auto GetModule() -> sh::game::ComponentModule*
	{
		return &module;
	}
}