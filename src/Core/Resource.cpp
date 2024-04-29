#include "Resource.h"

namespace sh::core
{
	Resource::Resource(unsigned int handle) :
		handle(handle)
	{
	}

	Resource::Resource(const Resource& other) :
		handle(other.handle)
	{
	}

	Resource::Resource(Resource&& other) noexcept :
		handle(other.handle)
	{
	}

	auto Resource::operator=(const Resource& other) -> Resource&
	{
		handle = other.handle;

		return *this;
	}

	auto Resource::operator=(Resource&& other) noexcept -> Resource&
	{
		handle = other.handle;

		return *this;
	}

	auto Resource::GetHandle() const -> int
	{
		 return handle;
	}
}