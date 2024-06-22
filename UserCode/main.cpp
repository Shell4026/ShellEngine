#include "Export.h"

#include "ComponentTest.h"
#include "RotateObject.h"

#include "Game/ComponentModule.h"

#include <iostream>

extern "C"
{
	SH_USER_API void Init()
	{
		std::cout << "Init User module.\n";

		REGISTER_COMPONENT(ComponentTest);
		REGISTER_COMPONENT(RotateObject);
	}
	SH_USER_API auto GetModule() -> sh::game::ComponentModule*
	{
		return ComponentModule::GetInstance();
	}
}