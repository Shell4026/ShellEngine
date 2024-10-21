#include "Reflection.hpp"

namespace sh::core::reflection
{
	SHashMap<uint32_t, STypeInfo*> STypes::types{};

	auto STypeInfo::GetSuper() const -> const STypeInfo*
	{
		return super;
	}

	bool STypeInfo::IsA(const STypeInfo& other) const
	{
		if (this == &other)
			return true;

		//다른 dll 영역에 올라갈 경우 같은 클래스여도 주소가 다르다. 따라서 Hash값으로 비교한다.
		return this->hash == other.hash;
	}

	bool STypeInfo::operator==(const STypeInfo& other) const
	{
		return IsA(other);
	}

	bool STypeInfo::IsChildOf(const STypeInfo& other) const
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

	auto STypeInfo::AddProperty(const std::string& name, const Property& prop) -> Property*
	{
		auto it = properties.insert({ name, prop });
		if (!it.second)
			return nullptr;
		return &it.first->second;
	}

	auto STypeInfo::GetProperty(const std::string& name) -> Property*
	{
		auto it = properties.find(name);
		if (it == properties.end())
			return nullptr;
		return &it->second;
	}

	auto STypeInfo::GetProperties() const -> const std::map<std::string, Property>&
	{
		return properties;
	}

	void STypeInfo::AddSObjectPtrProperty(Property* prop)
	{
		pointers.push_back(prop);
	}
	auto STypeInfo::GetSObjectPtrProperties() const -> const std::vector<Property*>&
	{
		return pointers;
	}

	void STypeInfo::AddSObjectContainerProperty(Property* prop)
	{
		containers.push_back(prop);
	}
	auto STypeInfo::GetSObjectContainerProperties() const -> const std::vector<Property*>&
	{
		return containers;
	}

	auto PropertyDataBase::GetTypeName() const -> std::string_view
	{
		return typeName;
	}

	Property::Property(PropertyDataBase* data, const char* name, bool isContainer, uint32_t containerNestedLevel) :
		data(data), name(name),
		bConstProperty(data->bConst), bVisibleProperty(data->bVisible),
		isContainer(isContainer), containerNestedLevel(containerNestedLevel), isPointer(data->isPointer),
		isSObject(data->isSObject), isSObjectPointer(data->isSObjectPointer), isConst(data->isConst)
	{

	}

	auto Property::GetName() const -> std::string_view
	{
		return name;
	}

	auto Property::GetTypeName() const -> std::string_view
	{
		return data->GetTypeName();
	}

	auto Property::Begin(SObject* sobject) -> PropertyIterator
	{
		return data->Begin(sobject);
	}

	auto Property::End(SObject* sobject) -> PropertyIterator
	{
		return data->End(sobject);
	}

	PropertyIterator::PropertyIterator()
	{
	}

	PropertyIterator::PropertyIterator(PropertyIterator&& other) noexcept :
		iteratorData(std::move(other.iteratorData))
	{
	}

	PropertyIterator::PropertyIterator(std::unique_ptr<IPropertyIteratorBase>&& iteratorData) :
		iteratorData(std::move(iteratorData))
	{
	}

	auto PropertyIterator::operator=(PropertyIterator&& other) noexcept -> PropertyIterator&
	{
		iteratorData = std::move(other.iteratorData);
		return *this;
	}

	auto PropertyIterator::operator==(const PropertyIterator & other) -> bool
	{
		return *iteratorData.get() == *other.iteratorData.get();
	}

	auto PropertyIterator::operator!=(const PropertyIterator& other) -> bool
	{
		return *iteratorData.get() != *other.iteratorData.get();
	}

	auto PropertyIterator::operator++() -> PropertyIterator&
	{
		++(*iteratorData.get());
		return *this;
	}

	auto PropertyIterator::GetTypeName() const -> std::string_view
	{
		return iteratorData->typeName;
	}

	auto PropertyIterator::GetNestedBegin() -> PropertyIterator
	{
		return iteratorData->GetNestedBegin();
	}

	auto PropertyIterator::GetNestedEnd() -> PropertyIterator
	{
		return iteratorData->GetNestedEnd();
	}

	auto PropertyIterator::IsPair() const -> bool
	{
		return iteratorData->IsPair();
	}
	auto PropertyIterator::IsConst() const -> bool
	{
		return iteratorData->IsConst();
	}

	void PropertyIterator::Erase()
	{
		iteratorData->Erase();
	}
;}