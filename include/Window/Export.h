#pragma once

#include "../Core/Config.h"

#ifdef ShellEngineWindow_EXPORTS
#define SH_WINDOW_API SH_API_EXPORT
#else
#define SH_WINDOW_API SH_API_IMPORT
#endif