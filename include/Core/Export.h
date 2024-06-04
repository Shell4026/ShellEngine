#pragma once

#include "Config.h"
#ifdef ShellEngineCore_EXPORTS
#define SH_CORE_API SH_API_EXPORT
#else
#define SH_CORE_API SH_API_IMPORT
#endif