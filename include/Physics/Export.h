#pragma once

#include "../Core/Config.h"

#ifdef ShellEnginePhysics_EXPORTS
#define SH_PHYS_API SH_API_EXPORT
#else
#define SH_PHYS_API SH_API_IMPORT
#endif