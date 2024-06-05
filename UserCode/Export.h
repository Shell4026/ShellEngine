#include "Core/Config.h"

#ifdef ShellEngineUser_EXPORTS
#define SH_USER_API SH_API_EXPORT
#else
#define SH_USER_API SH_API_IMPORT
#endif