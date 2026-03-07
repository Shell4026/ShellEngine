#pragma once
#include "../Core/Config.h"

#ifdef ShellEngineSound_EXPORTS
#define SH_SOUND_API SH_API_EXPORT
#else
#define SH_SOUND_API SH_API_IMPORT
#endif