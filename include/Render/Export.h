#pragma once

#ifndef GLM_FORCE_SSE2
#define GLM_FORCE_SSE2
#endif

#include "../Core/Config.h"

#ifdef ShellEngineRender_EXPORTS
#define SH_RENDER_API SH_API_EXPORT
#else
#define SH_RENDER_API SH_API_IMPORT
#endif