#pragma once
#include "Reflection/Property.hpp"

namespace sh::core::reflection
{
	Property::Property(const Property& other) noexcept :
		data(other.data),
		type(other.type),
		name(other.name),
		containerNestedLevel(other.containerNestedLevel),
		bConstProperty(other.bConstProperty),
		bVisibleProperty(other.bVisibleProperty),
		isConst(other.isConst),
		isPointer(other.isPointer),
		isContainer(other.isContainer),
		isSObject(other.isSObject),
		isSObjectPointer(other.isSObjectPointer),
		isSObjectPointerContainer(other.isSObjectPointerContainer)
	{
	}
	SH_CORE_API auto Property::GetName() const -> const core::Name&
	{
		return name;
	}
	SH_CORE_API auto Property::Begin(SObject* SObject) -> PropertyIterator
	{
		return data->Begin(SObject);
	}
	SH_CORE_API auto Property::Begin(SObject* SObject) const -> PropertyIterator
	{
		return data->Begin(SObject);
	}

	SH_CORE_API auto Property::End(SObject* SObject) -> PropertyIterator
	{
		return data->End(SObject);
	}
	SH_CORE_API auto Property::End(SObject* SObject) const -> PropertyIterator
	{
		return data->End(SObject);
	}
	SH_CORE_API auto Property::GetContainerNestedLevel() const -> uint32_t
	{
		return containerNestedLevel;
	}
}//namespace