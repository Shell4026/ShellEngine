#pragma once

#include "Export.h"

#include <string_view>

namespace sh::core
{
	class ModuleLoader
	{
	public:
		SH_CORE_API auto Load(std::string_view moduleName) -> void*;
	};
}