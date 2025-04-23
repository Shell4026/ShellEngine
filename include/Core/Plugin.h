#pragma once
#include "Export.h"
#include "Reflection.hpp"

#include <string>
#include <filesystem>
namespace sh::core
{
	struct Plugin
	{
		using Handle = void*;

		Handle handle = nullptr;
		std::string name;
		std::filesystem::path path;
	};
}//namespace