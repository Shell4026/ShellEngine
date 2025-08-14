#include "AssetResolver.h"

namespace sh::core
{
	AssetResolverFn AssetResolverRegistry::resolver = nullptr;

	SH_CORE_API void AssetResolverRegistry::SetResolver(AssetResolverFn r)
	{
		resolver = r;
	}
	SH_CORE_API auto AssetResolverRegistry::GetResolver() -> AssetResolverFn&
	{
		return resolver;
	}
}
