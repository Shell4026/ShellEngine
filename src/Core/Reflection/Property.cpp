#pragma once
#include "Reflection/Property.hpp"

namespace sh::core::reflection
{
	Property::Property(const Property& other) noexcept :
		data(other.data),
		pureTypeName(other.pureTypeName),
		type(other.type),
		name(other.name),
		containerNestedLevel(other.containerNestedLevel),
		containerElementType(other.containerElementType),
		bConstProperty(other.bConstProperty),
		bVisibleProperty(other.bVisibleProperty),
		bNoSaveProperty(other.bNoSaveProperty),
		isConst(other.isConst),
		isPointer(other.isPointer),
		isContainer(other.isContainer),
		isSObject(other.isSObject),
		isSObjectPointer(other.isSObjectPointer),
		isSObjectPointerContainer(other.isSObjectPointerContainer),
		isEnum(other.isEnum)
	{
	}
	SH_CORE_API auto Property::operator==(const Property& other) -> bool
	{
		return type == other.type && isConst == other.isConst;
	}
	SH_CORE_API auto Property::operator==(const TypeInfo& type) -> bool
	{
		return this->type == type && isConst == type.isConst;
	}
	SH_CORE_API auto Property::operator!=(const Property& other) -> bool
	{
		return !operator==(other);
	}
	SH_CORE_API auto Property::operator!=(const TypeInfo& type) -> bool
	{
		return !operator==(type);
	}

	SH_CORE_API auto Property::GetName() const -> const core::Name&
	{
		return name;
	}
	SH_CORE_API auto Property::Begin(SObject& SObject) const -> PropertyIteratorT
	{
		return data->Begin(&SObject);
	}
	SH_CORE_API auto Property::Begin(const SObject& SObject) const -> ConstPropertyIteratorT
	{
		return data->Begin(&SObject);
	}
	SH_CORE_API auto Property::End(SObject& SObject) const -> PropertyIteratorT
	{
		return data->End(&SObject);
	}
	SH_CORE_API auto Property::End(const SObject& SObject) const -> ConstPropertyIteratorT
	{
		return data->End(&SObject);
	}
	SH_CORE_API auto Property::GetContainerNestedLevel() const -> uint32_t
	{
		return containerNestedLevel;
	}
	SH_CORE_API auto Property::ClearContainer(SObject& SObject) const -> bool
	{
		if (isContainer)
		{
			data->ClearContainer(&SObject);
		}
		return false;
	}

}//namespace