#pragma once

#include "Export.h"

#include <vector>
#include <string_view>
#include <optional>

namespace sh::core
{
	class FileLoader
	{
	public:
		SH_CORE_API virtual ~FileLoader();

		SH_CORE_API auto LoadBinary(std::string_view dir)->std::optional<std::vector<unsigned char>>;
		SH_CORE_API auto LoadText(std::string_view dir) -> std::optional<std::string>;
	};
}