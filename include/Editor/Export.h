﻿#pragma once

#include "Core/Config.h"

#ifdef ShellEngineEditor_EXPORTS
#define SH_EDITOR_API SH_API_EXPORT
#else
#define SH_EDITOR_API SH_API_IMPORT
#endif