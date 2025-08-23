#pragma once
#include "Reflection/Property.hpp"

namespace sh::core::reflection
{
	PropertyIterator::PropertyIterator()
	{
		std::memset(iteratorBuffer, 0, sizeof(iteratorBuffer));
	}
	PropertyIterator::PropertyIterator(const PropertyIterator& other)
	{
		other.iteratorDataPtr->Clone(iteratorBuffer);
		iteratorDataPtr = reinterpret_cast<IPropertyIteratorBase*>(iteratorBuffer);
	}
	PropertyIterator::~PropertyIterator()
	{
		if (iteratorDataPtr != nullptr)
			iteratorDataPtr->~IPropertyIteratorBase();
	}

	SH_CORE_API auto PropertyIterator::operator=(const PropertyIterator& other) -> PropertyIterator&
	{
		other.iteratorDataPtr->Clone(iteratorBuffer);
		iteratorDataPtr = reinterpret_cast<IPropertyIteratorBase*>(iteratorBuffer);
		return *this;
	}
	SH_CORE_API auto PropertyIterator::operator==(const PropertyIterator& other) -> bool
	{
		return *iteratorDataPtr == *other.iteratorDataPtr;
	}

	SH_CORE_API auto PropertyIterator::operator!=(const PropertyIterator& other) -> bool
	{
		return *iteratorDataPtr != *other.iteratorDataPtr;
	}

	SH_CORE_API auto PropertyIterator::operator++() -> PropertyIterator&
	{
		++(*iteratorDataPtr);
		return *this;
	}

	SH_CORE_API auto PropertyIterator::GetType() const -> const TypeInfo&
	{
		return iteratorDataPtr->GetType();
	}
	SH_CORE_API auto PropertyIterator::GetPairType() const -> std::optional<std::pair<TypeInfo, TypeInfo>>
	{
		return iteratorDataPtr->GetPairType();
	}

	SH_CORE_API auto PropertyIterator::GetNestedBegin() -> PropertyIterator
	{
		return iteratorDataPtr->GetNestedBegin();
	}

	SH_CORE_API auto PropertyIterator::GetNestedEnd() -> PropertyIterator
	{
		return iteratorDataPtr->GetNestedEnd();
	}

	SH_CORE_API auto PropertyIterator::IsPair() const -> bool
	{
		return iteratorDataPtr->IsPair();
	}
	SH_CORE_API auto PropertyIterator::IsConst() const -> bool
	{
		return iteratorDataPtr->IsConst();
	}

	SH_CORE_API void PropertyIterator::Erase()
	{
		iteratorDataPtr->Erase();
	}

	Property::Property(const Property& other) noexcept :
		data(other.data),
		pureTypeName(other.pureTypeName),
		type(other.type),
		name(other.name),
		containerNestedLevel(other.containerNestedLevel),
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
	SH_CORE_API auto Property::Begin(SObject& SObject) -> PropertyIterator
	{
		return data->Begin(&SObject);
	}
	SH_CORE_API auto Property::Begin(SObject& SObject) const -> PropertyIterator
	{
		return data->Begin(&SObject);
	}

	SH_CORE_API auto Property::End(SObject& SObject) -> PropertyIterator
	{
		return data->End(&SObject);
	}
	SH_CORE_API auto Property::End(SObject& SObject) const -> PropertyIterator
	{
		return data->End(&SObject);
	}
	SH_CORE_API auto Property::GetContainerNestedLevel() const -> uint32_t
	{
		return containerNestedLevel;
	}

}//namespace