#pragma once

#include "Export.h"

namespace sh::core
{
	class Resource
	{
	private:
		unsigned int handle;
	protected:
		SH_CORE_API Resource(unsigned int handle);
		SH_CORE_API Resource(const Resource& other);
		SH_CORE_API Resource(Resource&& other) noexcept;

		SH_CORE_API auto operator=(const Resource& other) -> Resource&;
		SH_CORE_API auto operator=(Resource&& other) noexcept -> Resource&;

		SH_CORE_API auto GetHandle() const -> int;
	};
}