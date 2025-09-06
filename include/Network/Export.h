#pragma once

#include "../Core/Config.h"

#ifdef ShellEngineNetwork_EXPORTS
#define SH_NET_API SH_API_EXPORT
#else
#define SH_NET_API SH_API_IMPORT
#endif