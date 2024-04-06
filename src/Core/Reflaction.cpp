#include "Reflaction.hpp"

namespace sh::core
{
	auto Reflection::TypeInfo::GetName() const -> const char*
	{
		return name;
	}

	auto Reflection::TypeInfo::GetSuper() const -> const TypeInfo*
	{
		return super;
	}

	bool Reflection::TypeInfo::IsA(const TypeInfo& other) const
	{
		if (this == &other)
			return true;

		return this->hash == other.hash;
	}

	bool Reflection::TypeInfo::IsChild(const TypeInfo& other) const
	{
		if (IsA(other))
			return true;

		const TypeInfo* super = GetSuper();
		while (super != nullptr)
		{
			if (super->IsA(other))
				return true;
			super = super->GetSuper();
		}
		return false;
	}
}