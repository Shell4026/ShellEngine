#pragma once

#include "Export.h"
#include "Reflection.hpp"

#include <vector>
#include <map>
#include <string>
#include <utility>

namespace sh::core
{
	class SObject
	{
		SCLASS(SObject)
	private:
		std::map<SObject*, int> refThis;
		bool isPendingKill;
	public:
		SH_CORE_API auto IsPendingKill() const -> bool;
		SH_CORE_API void Destroy();

		SH_CORE_API void UpdateRef();
	};

	SH_CORE_API bool IsValid(SObject* obj);
}