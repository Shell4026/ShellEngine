#pragma

#include "../Core/Config.h"

#ifdef ShellEngineRender_EXPORTS
#define SH_RENDER_API SH_API_EXPORT
#else
#define SH_RENDER_API SH_API_IMPORT
#endif