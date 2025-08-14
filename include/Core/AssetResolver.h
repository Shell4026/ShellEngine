#pragma once
#include "Export.h"
#include "UUID.h"
#include <functional>

namespace sh::core
{
	class SObject;

	using AssetResolverFn = std::function<SObject*(const UUID&)>;

	class AssetResolverRegistry
	{
	public:
		SH_CORE_API static void SetResolver(AssetResolverFn r);
		SH_CORE_API static auto GetResolver() -> AssetResolverFn&;
	private:
		SH_CORE_API static AssetResolverFn resolver;
	};
}
