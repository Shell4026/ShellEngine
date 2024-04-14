#pragma once

#include "Export.h"

#include <vector>
#include <string_view>
#include <optional>

namespace sh::core
{
	class FileLoader
	{
	protected:
		std::vector<unsigned char> data;
		std::string strData;
	public:
		SH_CORE_API virtual ~FileLoader();

		SH_CORE_API auto LoadText(std::string_view dir) -> std::optional<const std::reference_wrapper<std::string>>;
		SH_CORE_API auto LoadBinary(std::string_view dir) -> std::optional<const std::reference_wrapper<std::vector<unsigned char>>>;
	};
}