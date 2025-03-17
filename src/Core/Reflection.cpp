#include "Reflection.hpp"

namespace sh::core::reflection
{
	SHashMap<std::size_t, STypeInfo*> STypes::types{};

	SH_CORE_API auto STypeInfo::GetSuper() const -> const STypeInfo*
	{
		return super;
	}

	SH_CORE_API bool STypeInfo::IsA(const STypeInfo& other) const
	{
		if (this == &other)
			return true;
		// 다른 dll 영역에 올라갈 경우 같은 클래스여도 주소가 다르다. 따라서 Hash값으로 비교한다.
		return this->hash == other.hash;
	}

	SH_CORE_API bool STypeInfo::operator==(const STypeInfo& other) const
	{
		return IsA(other);
	}
	SH_CORE_API bool STypeInfo::operator!=(const STypeInfo& other) const
	{
		return !IsA(other);
	}

	SH_CORE_API bool STypeInfo::IsChildOf(const STypeInfo& other) const
	{
		if (IsA(other))
			return true;

		const STypeInfo* super = GetSuper();
		while (super != nullptr)
		{
			if (super->IsA(other))
				return true;
			super = super->GetSuper();
		}
		return false;
	}

	SH_CORE_API auto STypeInfo::AddProperty(const std::string& name, const Property& prop) -> Property*
	{
		auto it = properties.insert({ name, prop });
		if (!it.second)
			return nullptr;
		return &it.first->second;
	}

	SH_CORE_API auto STypeInfo::GetProperty(const std::string& name) -> Property*
	{
		auto it = properties.find(name);
		if (it == properties.end())
			return nullptr;
		return &it->second;
	}

	SH_CORE_API auto STypeInfo::GetProperties() const -> const std::map<std::string, Property>&
	{
		return properties;
	}

	SH_CORE_API void STypeInfo::AddSObjectPtrProperty(Property* prop)
	{
		pointers.push_back(prop);
	}
	SH_CORE_API auto STypeInfo::GetSObjectPtrProperties() const -> const std::vector<Property*>&
	{
		return pointers;
	}

	SH_CORE_API void STypeInfo::AddSObjectContainerProperty(Property* prop)
	{
		containers.push_back(prop);
	}
	SH_CORE_API auto STypeInfo::GetSObjectContainerProperties() const -> const std::vector<Property*>&
	{
		return containers;
	}

	SH_CORE_API Property::Property(PropertyDataBase* data, const char* name, bool isContainer, uint32_t containerNestedLevel) :
		data(data), name(name),
		bConstProperty(data->bConst), bVisibleProperty(data->bVisible),
		isContainer(isContainer), containerNestedLevel(containerNestedLevel), isPointer(data->isPointer),
		isSObject(data->isSObject), isSObjectPointer(data->isSObjectPointer), isConst(data->isConst),
		type(data->GetType())
	{

	}

	SH_CORE_API auto Property::GetName() const -> std::string_view
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