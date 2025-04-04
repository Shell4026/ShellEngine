#include "Reflection.hpp"

namespace sh::core::reflection
{
	SH_CORE_API PropertyIterator::PropertyIterator()
	{
	}

	SH_CORE_API PropertyIterator::PropertyIterator(PropertyIterator&& other) noexcept :
		iteratorData(std::move(other.iteratorData))
	{
	}

	SH_CORE_API PropertyIterator::PropertyIterator(std::unique_ptr<IPropertyIteratorBase>&& iteratorData) :
		iteratorData(std::move(iteratorData))
	{
	}

	SH_CORE_API auto PropertyIterator::operator=(PropertyIterator&& other) noexcept -> PropertyIterator&
	{
		iteratorData = std::move(other.iteratorData);
		return *this;
	}

	SH_CORE_API auto PropertyIterator::operator==(const PropertyIterator & other) -> bool
	{
		return *iteratorData.get() == *other.iteratorData.get();
	}

	SH_CORE_API auto PropertyIterator::operator!=(const PropertyIterator& other) -> bool
	{
		return *iteratorData.get() != *other.iteratorData.get();
	}

	SH_CORE_API auto PropertyIterator::operator++() -> PropertyIterator&
	{
		++(*iteratorData.get());
		return *this;
	}

	SH_CORE_API auto PropertyIterator::GetType() const -> TypeInfo
	{
		return iteratorData->GetType();
	}
	SH_CORE_API auto PropertyIterator::GetPairType() const -> std::optional<std::pair<TypeInfo, TypeInfo>>
	{
		return iteratorData->GetPairType();
	}

	SH_CORE_API auto PropertyIterator::GetNestedBegin() -> PropertyIterator
	{
		return iteratorData->GetNestedBegin();
	}

	SH_CORE_API auto PropertyIterator::GetNestedEnd() -> PropertyIterator
	{
		return iteratorData->GetNestedEnd();
	}

	SH_CORE_API auto PropertyIterator::IsPair() const -> bool
	{
		return iteratorData->IsPair();
	}
	SH_CORE_API auto PropertyIterator::IsConst() const -> bool
	{
		return iteratorData->IsConst();
	}

	SH_CORE_API void PropertyIterator::Erase()
	{
		iteratorData->Erase();
	}
;}