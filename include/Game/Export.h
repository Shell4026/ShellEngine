﻿#pragma once

#include "../Core/Config.h"

#ifdef ShellEngineGame_EXPORTS
#define SH_GAME_API SH_API_EXPORT
#else
#define SH_GAME_API SH_API_IMPORT
#endif