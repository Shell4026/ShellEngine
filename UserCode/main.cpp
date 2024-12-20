﻿#include "Export.h"
#include "Core/Logger.h"
#include "Game/ComponentModule.h"

extern "C"
{
	SH_USER_API void Init()
	{
		SH_INFO("Init User module.\n");
	}
	SH_USER_API auto GetModule() -> sh::game::ComponentModule*
	{
		return sh::game::ComponentModule::GetInstance();
	}
}